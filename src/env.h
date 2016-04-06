#ifndef SPARROW_ENV_H_
#define SPARROW_ENV_H_

#include <iostream>

#include <memory>
#include <string>
#include <map>
#include <vector>
#include <exception>
#include <memory>

#include "symbols.h"
#include "vm/code.h"

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

  //返回自己的一份拷贝,
  virtual std::shared_ptr<Object> copy() = 0;

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

  ObjectPtr copy() override{
    return std::make_shared<NoneObject>();
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

  ObjectPtr copy() override {
    return std::make_shared<IntObject>(value_);
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

  ObjectPtr copy() override {
    return std::make_shared<FloatObject>(value_);
  }
public:
  double value_;
};
using FloatObjectPtr = std::shared_ptr<FloatObject>;

/******************************Str 类型********************************/
class StrObject: public Object {
public:
  StrObject(const std::string &str): Object(ObjKind::STRING), str_(str) {}

  std::string info() override {
    return "Str: " + str_; 
  }

  ObjectPtr copy() override {
    return std::make_shared<StrObject>(str_);
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

  ObjectPtr copy() override {
    return std::make_shared<BoolObject>(b_);
  }
public:
  bool b_;
};
using BoolObjectPtr = std::shared_ptr<BoolObject>;

/*****************************函数 类型******************************/

class FuncObject;
using FuncPtr = std::shared_ptr<FuncObject>;

class FuncObject: public Object, public std::enable_shared_from_this<FuncObject>{

public:
  //指向当前正在编译的函数的指针
  static FuncPtr complingFunc;

  //获取当前正在编译的函数
  static FuncPtr getCurrCompilingFunc();

  //设置当前正在编译的函数
  static void setCurrCompilingFunc(FuncPtr func);

public:
  FuncObject(const std::string &functionName, size_t size, 
      std::shared_ptr<ParameterListAST> params, 
      std::shared_ptr<BlockStmntAST> block, EnvPtr env);

  //获得形参AST
  std::shared_ptr<ParameterListAST> params() const;

  //获得函数块AST
  std::shared_ptr<BlockStmntAST> block() const;

  //设置上层环境
  void setOuterEnv(EnvPtr env);

  //获取运行时环境
  EnvPtr runtimeEnv();
  
  std::string info() override {
    return "Func: " + funcName_;
  }

  //对代码块、外部名字不进行深度复制
  ObjectPtr copy() override;

  std::string funcName() const{
    return funcName_;
  }

  CodePtr getCodes() {
    return codes_;
  }

  //对于函数运行时所需的非局部变量，记录下它的名字并分配一个下标
  unsigned getRuntimeIndex(const std::string &name);

  std::shared_ptr<std::vector<std::string>> getOuterNames();

  //函数是否已经已经编译
  bool isCompile();

  //编译该函数
  void compile();

  //把函数对象的编译状态设为已编译
  void setCompiled();

private:
  std::string funcName_;

  //函数运行时环境局部变量所需空间大小
  size_t localVarSize_;

  //外部环境，一般作为函数运行时环境的上层
  //对于普通函数而言，它就是该unit的全局环境
  //对于类函数而言，它是某个实例的内部环境
  EnvPtr env_;

  //参数表和代码块的AST
  std::shared_ptr<ParameterListAST> params_;
  std::shared_ptr<BlockStmntAST> block_;

  //函数编译后的字节码
  CodePtr codes_;

  //非局部变量名称
  std::shared_ptr<std::vector<std::string>> outerNames_;

  //是否已经编译（在虚拟机运行时）
  //1.def的函数都是已编译的
  //2.在全局被引用的lamb函数（即全局环境中使用lamb定义或使用产生lamb的普通函数），
  //初次在虚拟机运行时，是未编译的（运行时被动编译，由虚拟机发现并调用FuncObj的编译）
  //3.未在全局引用过的lamb函数，初次在虚拟机运行时，
  //是未编译的（运行时主动编译，虚拟机找到Lamb源码并编译）
  bool isCompile_ = false;
};

/****************************原生函数********************************/

class NativeFunction: public Object, public std::enable_shared_from_this<NativeFunction> {
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

  //原生函数是没有副作用的，直接返回本身即可
  ObjectPtr copy() override {
    return shared_from_this();
  }

  //所有的原生函数子类通过实现该接口实现其功能
  virtual ObjectPtr invoke(const std::vector<ObjectPtr> &params) = 0;

protected:
  std::string funcName_;
  size_t paramNum_;
};
using NativeFuncPtr = std::shared_ptr<NativeFunction>;

/***************************类元信息*********************************/
class ClassInfo;
using ClassInfoPtr = std::shared_ptr<ClassInfo>;

class ClassInfo: public Object, public std::enable_shared_from_this<ClassInfo> {
public:
  ClassInfo(std::shared_ptr<ClassStmntAST> stmnt, EnvPtr env);

  //返回类名
  std::string name();

  //返回父类
  ClassInfoPtr superClass();

  //返回类定义体
  std::shared_ptr<ClassBodyAST> body();

  //获取类定义时所处的外部环境
  EnvPtr getEnvitonment();
  
  std::string info() override;

  //语言运行时一个类定义只产生一个类元信息，复制函数直接返回本身
  ObjectPtr copy() override {
    return shared_from_this();
  }

  //编译类中的每个函数
  //生成一个包含已编译的函数对象、变量的环境
  void compile();

  //获取该类编译后的环境
  EnvPtr getComliedEnv();

private:  
  std::shared_ptr<ClassStmntAST> definition_;
  EnvPtr outerEnv_;
  ClassInfoPtr superClass_ = nullptr;

  //变量、函数编译后所放在的环境
  //当类实例化时，把该环境拷贝给对象
  EnvPtr compiledEnv_ = nullptr;
};

/*****************************对象***********************************/

class ClassInstance: public Object, public std::enable_shared_from_this<ClassInstance> {
public:
  ClassInstance(EnvPtr env);

  void write(const std::string &member, ObjectPtr value);

  ObjectPtr read(const std::string &member);

  std::string info() override;

  //对象复制的语义交给语言使用者实现，此处的复制返回自身
  ObjectPtr copy() override {
    return shared_from_this();
  }

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

class Array: public Object, public std::enable_shared_from_this<Array> {
public:
  Array(size_t i);

  void set(size_t i, ObjectPtr obj);

  ObjectPtr get(size_t i);

  std::string info() override;

  //数组不能变更，直接返回自身
  ObjectPtr copy() override {
    return shared_from_this();
  }
private:
  std::vector<ObjectPtr> array_;
};
using ArrayPtr = std::shared_ptr<Array>;

/****************************环境***********************************/

//----------------------通用环境接口
class CommonEnv :public std::enable_shared_from_this<CommonEnv>, public Object{
public:
  
  CommonEnv();

  CommonEnv(EnvPtr outer);

  //设置外部环境
  void setOuterEnv(EnvPtr outer);

  //获取上层环境
  EnvPtr getOuterEnv() const;

  //由内到外搜寻获取环境内指定变量，如果找不到抛出异常
  //对于$开头的全局变量，只从最外层的环境查找
  virtual ObjectPtr get(const std::string &name) = 0;

  //获取指定位置的变量
  virtual ObjectPtr get(size_t index) = 0;

  //把变量放入环境中
  //如果是新的变量，或在当前环境中可以找到该环境，那么就放入该环境
  //否则向外寻找变量所在的环境，并更新它的值
  virtual void put(const std::string &name, ObjectPtr obj) = 0;

  //向环境中指定位置放入变量
  //只用于函数运行时环境的局部变量，需要事先确定局部变量的位置
  virtual void put(size_t index, ObjectPtr obj) = 0;

  //检查变量是否在当前的环境
  virtual bool isExistInCurrentEnv(const std::string &name) = 0;

  //获取当前环境的指定值
  virtual ObjectPtr getCurr(const std::string &name) = 0;

  //向当前环境插入新的变量
  virtual void putCurr(const std::string &name, ObjectPtr obj) = 0;

  SymbolsPtr getUnitSymbols() const {
    return unitSymbols_;
  }

  void setUnitSymbols(SymbolsPtr symbols) {
    unitSymbols_ = symbols;
  }

protected:
  //定位一个变量的位置
  //如果该变量不存在于任何环境，则返回空指针
  EnvPtr locateEnv(const std::string &name);

  //获取最外层的环境
  EnvPtr getOutestEnv();

protected:
  EnvPtr outerEnv_ = nullptr;
  
  //当前环境的符号表
  SymbolsPtr unitSymbols_;
};

//----------------------使用map实现的环境
//----------------------全局环境、对象环境
class MapEnv: public CommonEnv {
public:
  MapEnv();

  MapEnv(EnvPtr outer);

  ObjectPtr get(const std::string &name) override;

  ObjectPtr get(size_t index) override;

  void put(const std::string &name, ObjectPtr obj) override;

  void put(size_t index, ObjectPtr obj) override;

  bool isExistInCurrentEnv(const std::string &name) override;

  ObjectPtr getCurr(const std::string &name) override;
  
  void putCurr(const std::string &name, ObjectPtr obj) override;

  std::string info() override;

  //该复制只用在类元实例化出对象
  //需要对环境内的各各对象进行深度赋值，且函数的外部环境更改为结果环境
  ObjectPtr copy() override;

private:
  std::map<std::string, ObjectPtr> values_;
};
using MapEnvPtr = std::shared_ptr<MapEnv>;

//---------------------使用数组实现的环境
//---------------------函数运行时的局部环境
class ArrayEnv: public CommonEnv {
public:
  ArrayEnv(EnvPtr outer, FuncPtr function, size_t size);

  ObjectPtr get(const std::string &name) override;

  ObjectPtr get(size_t index) override;

  void put(const std::string &name, ObjectPtr obj) override;

  void put(size_t index, ObjectPtr obj) override;

  bool isExistInCurrentEnv(const std::string &name) override;

  ObjectPtr getCurr(const std::string &name) override;
  
  void putCurr(const std::string &name, ObjectPtr obj) override;

  std::string info() override;

  //该环境没有复制的意义，抛出异常
  ObjectPtr copy() override;

private:
  std::weak_ptr<FuncObject> function_;

  std::string funcName_;

  std::vector<ObjectPtr> values_;
};
using ArrayEnvPtr = std::shared_ptr<ArrayEnv>;

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

class EnvNotFoundException: public EnvException {
public:
  EnvNotFoundException(const std::string &notFoundVarName):
    EnvException("Not found the variable: " + notFoundVarName) {};
};

#endif
