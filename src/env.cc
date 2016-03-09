#include "env.h"
#include "ast_list.h"

#include "debugger.h"

/*****************************函数 类型******************************/
FuncObject::FuncObject(const std::string &functionName, size_t size,
    std::shared_ptr<ParameterListAST> params, 
    std::shared_ptr<BlockStmntAST> block, EnvPtr env):
  Object(ObjKind::FUNCTION), funcName_(functionName), localVarSize_(size), 
  env_(env), params_(params), block_(block){}

std::shared_ptr<ParameterListAST> FuncObject::params() const {
  return params_;
}

std::shared_ptr<BlockStmntAST> FuncObject::block() const {
  return block_; 
}

EnvPtr FuncObject::runtimeEnv() {
  return std::make_shared<ArrayEnv>(env_, shared_from_this(), localVarSize_);
}

/***************************类元信息*********************************/
ClassInfo::ClassInfo(std::shared_ptr<ClassStmntAST> stmnt, EnvPtr env):
  Object(ObjKind::CLASS_INFO), definition_(stmnt), env_(env) {

  std::string superClassName = stmnt->superClassName();
  if (superClassName.empty())
    return;

  try {
    ObjectPtr obj = env->get(superClassName);
    if (obj->kind_ == ObjKind::CLASS_INFO)
      superClass_ = std::static_pointer_cast<ClassInfo>(obj);
    else
      throw EnvException(superClassName + " is not a valid class");   
  }
  catch (EnvException e) {
    throw EnvException("not found super class: " + stmnt->superClassName());
  }
}

std::string ClassInfo::name() {
  return definition_->name();
}

ClassInfoPtr ClassInfo::superClass() {
  return superClass_;
}

std::shared_ptr<ClassBodyAST> ClassInfo::body() {
  return definition_->body();
}

EnvPtr ClassInfo::getEnvitonment() {
  return env_;
}

std::string ClassInfo::info() {
  return "<class: " + name() + ">";
}

/**************************对象*************************************/

ClassInstance::ClassInstance(EnvPtr env): Object(ObjKind::CLASS_INSTANCE), env_(env){}

void ClassInstance::write(const std::string &member, ObjectPtr value) {
  //if (checkAccessValid(member))
  //允许自由改变对象内部的值
  env_->put(member, value);
}

ObjectPtr ClassInstance::read(const std::string &member) {
  //checkAccessValid(member);
  try {
    ObjectPtr result = env_->get(member);
    return result;
  }
  catch (EnvException e) {
    throw EnvException("not found variable " + member + " in object");
  }
}

bool ClassInstance::checkAccessValid(const std::string &member) {
  if (env_->isExistInCurrentEnv(member))
    return true;
  else
    throw EnvException("Access " + member + " error");
}

std::string ClassInstance::info() {
  return "<object>";
}

/*************************定长数组***********************************/

Array::Array(size_t i): Object(ObjKind::Array) {
  array_ = std::vector<ObjectPtr>(i, nullptr);
}

void Array::set(size_t i, ObjectPtr obj) {
  if (i >= array_.size())
    throw OutOfIndexException();
  else
    array_[i] = obj;
}

ObjectPtr Array::get(size_t i) {
  if (i >= array_.size())
    throw OutOfIndexException();
  else
    return array_[i];
}

std::string Array::info() {
  return "<array with size " + std::to_string(array_.size()) + ">";
}

/*******************************环境********************************/

//-----------------------环境接口

CommonEnv::CommonEnv(): CommonEnv(nullptr) {}

CommonEnv::CommonEnv(EnvPtr outer):Object(ObjKind::ENV), outerEnv_(outer) {}

void CommonEnv::setOuterEnv(EnvPtr outer) {
  outerEnv_ = outer;
}

EnvPtr CommonEnv::getOuterEnv() const {
  return outerEnv_;
}

EnvPtr CommonEnv::locateEnv(const std::string &name) {
  if (name.empty())
    return nullptr;

  //去最外层环境寻找$变量
  if (name[0] == '$') {
    auto outest = getOutestEnv();
    if (outest->isExistInCurrentEnv(name))
      return outest;
    else
      return nullptr;
  }

  //普通变量查找
  else {
    if (isExistInCurrentEnv(name))
      return this->shared_from_this();
    else if (outerEnv_ == nullptr)
      return nullptr;
    else
      return outerEnv_->locateEnv(name);
  }
}

EnvPtr CommonEnv::getOutestEnv() {
  if (outerEnv_ != nullptr)
    return outerEnv_->getOutestEnv();
  else
    return shared_from_this();
}


//----------------------全局环境、对象环境

MapEnv::MapEnv(): CommonEnv(nullptr) {}

MapEnv::MapEnv(EnvPtr outer): CommonEnv(outer) {}

ObjectPtr MapEnv::get(const std::string &name) {
  //用于对象环境的self语义
  if (name == "self")
    return shared_from_this();

  auto env = locateEnv(name);
  if (env == nullptr)
    throw EnvException("not found the variable: " + name);
  else 
    return env->getCurr(name);
}

ObjectPtr MapEnv::get(size_t __attribute__((unused))index) {
  throw ASTEvalException("can not get object by index in map envrionment");
}

void MapEnv::put(const std::string &name, ObjectPtr obj) {
  auto env = locateEnv(name);
  if (env == nullptr)
    env = this->shared_from_this();
  env->putCurr(name, obj);
}

void MapEnv::put(size_t __attribute__((unused))index, 
    ObjectPtr __attribute__((unused))obj) {
  throw ASTEvalException("can not put object by index in map environment");
}

bool MapEnv::isExistInCurrentEnv(const std::string &name) {
  return values_.find(name) != values_.end();
}

ObjectPtr MapEnv::getCurr(const std::string &name) {
  return values_[name];
}

void MapEnv::putCurr(const std::string &name, ObjectPtr obj) {
  values_[name] = obj;
}

std::string MapEnv::info() {
  return "MAP ENVIRONMENT";
}

//---------------------函数运行时局部环境

ArrayEnv::ArrayEnv(EnvPtr outer, FuncPtr function, size_t size): 
  CommonEnv(outer) {
  function_ = function;
  values_.clear();
  for (size_t i = 0; i < size; ++i)
    values_.push_back(nullptr);
  if (function_ == nullptr)
    throw ASTEvalException("function invalid while construct runtime env");
}

ObjectPtr ArrayEnv::get(const std::string &name) {
  //如果是函数调用自己，则直接返回
  if (name == function_->funcName())
    return function_;

  auto env = locateEnv(name);
  if (env == nullptr)
    throw EnvException("not found the variable: " + name);
  else 
    return env->getCurr(name);
}

ObjectPtr ArrayEnv::get(size_t index) {
  if (index >= values_.size())
    throw ASTEvalException("invalid index while getting obj");
  return values_[index];
}

void ArrayEnv::put(const std::string &__attribute__((unused))name, 
    ObjectPtr __attribute__((unused))obj) {
  throw ASTEvalException("can not put object by name in array environment");
}

void ArrayEnv::put(size_t index, ObjectPtr obj) {
  if (index >= values_.size())
    throw ASTEvalException("invalid index while putting obj");
  values_[index] = obj;
}

bool ArrayEnv::isExistInCurrentEnv(const std::string 
    &__attribute__((unused))name) {
  return false;
}

ObjectPtr ArrayEnv::getCurr(const std::string &__attribute__((unused))name) {
  throw ASTEvalException("can not get object by name in array environment");
}

void ArrayEnv::putCurr(const std::string &__attribute__((unused))name, 
    ObjectPtr __attribute__((unused))obj) {
  throw ASTEvalException("can not put object by name in array environment");
}

std::string ArrayEnv::info() {
  return "ARRAY ENVIRONMENT";
}
