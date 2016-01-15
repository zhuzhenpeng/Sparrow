#ifndef SPARROW_ENV_H_
#define SPARROW_ENV_H_

#include <memory>
#include <string>
#include <map>

class ParameterListAST;
class BlockStmntAST;
class Environment;
using EnvPtr = std::shared_ptr<Environment>;

/******************************解析时变量类型****************************/
enum class ObjKind {
  Int = 1, 
  String = 2, 
  Bool = 3,
  Func = 4
};

class Object {
public:
  Object(ObjKind kind): kind_(kind) {}
public:
  ObjKind kind_;
};
using ObjectPtr = std::shared_ptr<Object>;

/******************************Int 类型 ********************************/
class IntObject: public Object {
public:
  IntObject(int value): Object(ObjKind::Int), value_(value) {}
public:
  int value_;
};
using IntObjectPtr = std::shared_ptr<IntObject>;

/******************************Str 类型********************************/
class StrObject: public Object {
public:
  StrObject(const std::string &str): Object(ObjKind::String), str_(str) {}
public:
  std::string str_;
};
using StrObjectPtr = std::shared_ptr<StrObject>;

/*****************************BOOL 类型*******************************/
class BoolObject: public Object {
public:
  BoolObject(bool b): Object(ObjKind::Bool), b_(b) {}
  BoolObject(int num): Object(ObjKind::Bool), b_(num != 0) {}
public:
  bool b_;
};
using BoolObjectPtr = std::shared_ptr<BoolObject>;

/*****************************函数 类型******************************/

class FuncObject: public Object {
public:
  FuncObject(std::shared_ptr<ParameterListAST> params, 
      std::shared_ptr<BlockStmntAST> block, EnvPtr env);
  std::shared_ptr<ParameterListAST> params() const;
  std::shared_ptr<BlockStmntAST> block() const;
  EnvPtr runtimeEnv() const;

private:
  std::shared_ptr<ParameterListAST> params_;
  std::shared_ptr<BlockStmntAST> block_;
  EnvPtr env_;
};
using FuncPtr = std::shared_ptr<FuncObject>;

/********************************环境********************************/

class Environment :public std::enable_shared_from_this<Environment>{
public:
  Environment();

  Environment(EnvPtr outer);

  void setOuterEnv(EnvPtr outer);

  //由内到外搜寻获取环境内指定变量，如果找不到则返回空
  ObjectPtr get(const std::string &name);

  //把变量放入环境中
  //如果是新的变量，或在当前环境中可以找到该环境，那么就放入该环境
  //否则向外寻找变量所在的环境，并更新它的值
  void put(const std::string &name, ObjectPtr obj);

private:
  //定位变量所在环境，如果找不到则返回空
  EnvPtr locateEnv(const std::string &name);

  //插入新的变量
  void putNew(const std::string &name, ObjectPtr obj);

private:
  //外层环境
  EnvPtr outerEnv_ = nullptr;

  //当前环境的键值
  std::map<std::string, ObjectPtr> env_;
};

#endif
