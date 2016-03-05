#ifndef SPARROW_ENV_H_
#define SPARROW_ENV_H_

#include <iostream>

#include <memory>
#include <string>
#include <map>
#include <vector>
#include <exception>

class ParameterListAST;
class BlockStmntAST;
class ClassStmntAST;
class ClassBodyAST;
class CommonEnv;
using EnvPtr = std::shared_ptr<CommonEnv>;

/******************************解析时变量类型****************************/
enum class ObjKind {
  ENV = 0,
  NONE = 1,
  INT = 2, 
  FLOAT = 3,
  STRING = 4, 
  BOOL = 5,
  FUNCTION = 6,
  NATIVE_FUNC = 7,
  CLASS_INFO = 8,
  CLASS_INSTANCE = 9,
  Array = 10
};

class Object {
public:
  Object(ObjKind kind): kind_(kind) {}
  virtual std::string info() = 0;
public:
  ObjKind kind_;
};
using ObjectPtr = std::shared_ptr<Object>;

/******************************空类型***********************************/

class NoneObject: public Object {
public:
  NoneObject(): Object(ObjKind::NONE) {}
  std::string info() override {
    return "None";
  }
};
using NoneObjectPtr = std::shared_ptr<NoneObject>;

/******************************Int 类型 ********************************/
class IntObject: public Object {
public:
  IntObject(int value): Object(ObjKind::INT), value_(value) {}
  std::string info() override {
    return "Int: " + std::to_string(value_);
  }
public:
  int value_;
};
using IntObjectPtr = std::shared_ptr<IntObject>;

/*****************************Float类型********************************/
class FloatObject: public Object {
public:
  FloatObject(double value): Object(ObjKind::FLOAT), value_(value) {}
  std::string info() override {
    return "Float: " + std::to_string(value_);
  }
public:
  double value_;
};

/******************************Str 类型********************************/
class StrObject: public Object {
public:
  StrObject(const std::string &str): Object(ObjKind::STRING), str_(str) {}
  std::string info() override {
    return "Str: " + str_; 
  }
public:
  std::string str_;
};
using StrObjectPtr = std::shared_ptr<StrObject>;

/*****************************BOOL 类型*******************************/
class BoolObject: public Object {
public:
  BoolObject(bool b): Object(ObjKind::BOOL), b_(b) {}
  BoolObject(int num): Object(ObjKind::BOOL), b_(num != 0) {}
  std::string info() override {
    return "Bool: " + std::to_string(b_);
  }
public:
  bool b_;
};
using BoolObjectPtr = std::shared_ptr<BoolObject>;

/*****************************函数 类型******************************/

class FuncObject: public Object {
public:
  FuncObject(const std::string &functionName, std::shared_ptr<ParameterListAST> params, 
      std::shared_ptr<BlockStmntAST> block, EnvPtr env);
  std::shared_ptr<ParameterListAST> params() const;
  std::shared_ptr<BlockStmntAST> block() const;
  EnvPtr runtimeEnv() const;
  
  std::string info() override {
    return "Func: " + funcName_;
  }
private:
  std::string funcName_;
  std::shared_ptr<ParameterListAST> params_;
  std::shared_ptr<BlockStmntAST> block_;

  //外部环境，一般作为函数运行时环境的上层
  //对于普通函数而言，它就是该unit的全局环境
  //对于类函数而言，它是某个实例的内部环境
  EnvPtr env_;
};
using FuncPtr = std::shared_ptr<FuncObject>;

/****************************原生函数********************************/

class NativeFunction: public Object {
public:
  NativeFunction(const std::string &name, size_t paramNum): 
    Object(ObjKind::NATIVE_FUNC), funcName_(name), paramNum_(paramNum) {}

  std::string name() const {
    return funcName_;
  }

  size_t paramNum() const {
    return paramNum_;
  }

  std::string info() override {
    return "native function: " + funcName_;
  }

  virtual ObjectPtr invoke(const std::vector<ObjectPtr> &params) = 0;

protected:
  std::string funcName_;
  size_t paramNum_;
};
using NativeFuncPtr = std::shared_ptr<NativeFunction>;

/***************************类元信息*********************************/
class ClassInfo;
using ClassInfoPtr = std::shared_ptr<ClassInfo>;

class ClassInfo: public Object {
public:
  ClassInfo(std::shared_ptr<ClassStmntAST> stmnt, EnvPtr env);

  //返回类名
  std::string name();

  //返回父类
  ClassInfoPtr superClass();

  //返回类定义体
  std::shared_ptr<ClassBodyAST> body();

  EnvPtr getEnvitonment();
  
  std::string info() override;
private:  
  std::shared_ptr<ClassStmntAST> definition_;
  EnvPtr env_;
  ClassInfoPtr superClass_ = nullptr;
};

/*****************************对象***********************************/

class ClassInstance: public Object {
public:
  ClassInstance(EnvPtr env);

  void write(const std::string &member, ObjectPtr value);

  ObjectPtr read(const std::string &member);

  std::string info() override;

  EnvPtr getEnvironment() {
    return env_;
  }

private:
  bool checkAccessValid(const std::string &member);
private:
  EnvPtr env_;
};
using InstancePtr = std::shared_ptr<ClassInstance>;

/*************************定长数组***********************************/

class Array: public Object {
public:
  Array(size_t i);
  void set(size_t i, ObjectPtr obj);
  ObjectPtr get(size_t i);
  std::string info() override;
private:
  std::vector<ObjectPtr> array_;
};
using ArrayPtr = std::shared_ptr<Array>;

/****************************环境***********************************/

//通用环境，map记录变量名和下标，值存储在数组之中
class CommonEnv :public std::enable_shared_from_this<CommonEnv>, public Object{
public:
  CommonEnv();

  CommonEnv(EnvPtr outer);

  void setOuterEnv(EnvPtr outer);

  //由内到外搜寻获取环境内指定变量，如果找不到抛出异常
  //对于$开头的全局变量，只从最外层的环境查找
  ObjectPtr get(const std::string &name);

  //把变量放入环境中
  //如果是新的变量，或在当前环境中可以找到该环境，那么就放入该环境
  //否则向外寻找变量所在的环境，并更新它的值
  void put(const std::string &name, ObjectPtr obj);

  //检查变量是否在当前的环境
  bool isExistInCurrentEnv(const std::string &name);

  //返回变量在当前环境的下表，如果不存在于当前环境，则返回-1
  int getSymbolsIndex(const std::string &name);

  std::string info() override;

private:
  //定位变量所在环境，如果找不到则返回空
  EnvPtr locateEnv(const std::string &name);

  //插入新的变量
  void putNew(const std::string &name, ObjectPtr obj);

private:
  //外层环境
  EnvPtr outerEnv_ = nullptr;

  //当前环境的键值
  std::map<std::string, size_t> index_;

  //存储值的数组
  std::vector<ObjectPtr> values_;
};

/********************************异常******************************/

class EnvException: public std::exception {
public:
  EnvException(const std::string &errMsg): errMsg_(errMsg) {}
  const char* what() const noexcept override {
    return errMsg_.c_str();
  }
private:
  std::string errMsg_;
};

class OutOfIndexException: public EnvException {
public:
  OutOfIndexException(): EnvException("out of index") {};
};

#endif
