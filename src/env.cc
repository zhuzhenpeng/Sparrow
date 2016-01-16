#include "env.h"

/*****************************函数 类型******************************/
FuncObject::FuncObject(const std::string &functionName, 
    std::shared_ptr<ParameterListAST> params, 
    std::shared_ptr<BlockStmntAST> block, EnvPtr env):
  Object(ObjKind::Func), funcName_(functionName), params_(params), block_(block), env_(env) {}

std::shared_ptr<ParameterListAST> FuncObject::params() const {
  return params_;
}

std::shared_ptr<BlockStmntAST> FuncObject::block() const {
  return block_; 
}

EnvPtr FuncObject::runtimeEnv() const {
  return std::make_shared<Environment>(env_);
}

/********************************环境********************************/
Environment::Environment() = default;

Environment::Environment(EnvPtr outer): outerEnv_(outer) {}

void Environment::setOuterEnv(EnvPtr outer) {
  outerEnv_ = outer;
}

ObjectPtr Environment::get(const std::string &name) {
  auto env = locateEnv(name);
  if (env == nullptr)
    return nullptr;
  else 
    return env->env_[name];
}

void Environment::put(const std::string &name, ObjectPtr obj) {
  auto env = locateEnv(name);
  if (env == nullptr)
    env = this->shared_from_this();
  env->putNew(name, obj);
}

EnvPtr Environment::locateEnv(const std::string &name) {
  if (env_.find(name) != env_.end()) 
    return this->shared_from_this();
  else if (outerEnv_ == nullptr)
    return nullptr;
  else
    return outerEnv_->locateEnv(name);
}

void Environment::putNew(const std::string &name, ObjectPtr obj) {
  env_[name] = obj;
}
