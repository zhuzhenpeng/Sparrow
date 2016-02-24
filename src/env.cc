#include "env.h"
#include "ast_list.h"

/*****************************函数 类型******************************/
FuncObject::FuncObject(const std::string &functionName, 
    std::shared_ptr<ParameterListAST> params, 
    std::shared_ptr<BlockStmntAST> block, EnvPtr env):
  Object(ObjKind::FUNCTION), funcName_(functionName), params_(params), block_(block), env_(env) {}

std::shared_ptr<ParameterListAST> FuncObject::params() const {
  return params_;
}

std::shared_ptr<BlockStmntAST> FuncObject::block() const {
  return block_; 
}

EnvPtr FuncObject::runtimeEnv() const {
  return std::make_shared<CommonEnv>(env_);
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
  checkAccessValid(member);
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
CommonEnv::CommonEnv(): CommonEnv(nullptr) {};

CommonEnv::CommonEnv(EnvPtr outer):Object(ObjKind::ENV), outerEnv_(outer) {}

void CommonEnv::setOuterEnv(EnvPtr outer) {
  outerEnv_ = outer;
}

ObjectPtr CommonEnv::get(const std::string &name) {
  auto env = locateEnv(name);
  if (env == nullptr)
    throw EnvException("not found the variable: " + name);
  else 
    return env->env_[name];
}

void CommonEnv::put(const std::string &name, ObjectPtr obj) {
  auto env = locateEnv(name);
  if (env == nullptr)
    env = this->shared_from_this();
  env->putNew(name, obj);
}

bool CommonEnv::isExistInCurrentEnv(const std::string &name) {
  return env_.find(name) != env_.end();
}

std::string CommonEnv::info() {
  return "Enviroment";
}

EnvPtr CommonEnv::locateEnv(const std::string &name) {
  if (name.empty())
    return nullptr;

  //全局变量查找，只去全局环境中查找
  if (name[0] == '$') {
    if (outerEnv_ != nullptr) {   //只有全局环境才没有外部环境
      return outerEnv_->locateEnv(name);
    }
    else {
      if (env_.find(name) != env_.end())
        return this->shared_from_this();   
      else
        return nullptr;
    }
  }
  //普通变量查找
  else {
    if (env_.find(name) != env_.end()) 
      return this->shared_from_this();
    else if (outerEnv_ == nullptr)
      return nullptr;
    else
      return outerEnv_->locateEnv(name);
  }
}

void CommonEnv::putNew(const std::string &name, ObjectPtr obj) {
  env_[name] = obj;
}
