#ifndef SPARROW_AST_TREE_H_
#define SPARROW_AST_TREE_H_

#include <memory>
#include <string>
#include <map>
#include <exception>

/******************************解析时变量类型****************************/

enum class ObjKind {
  Int = 1, 
  String = 2, 
  Bool = 3
};

class Object {
public:
  Object(ObjKind kind): kind_(kind) {}
public:
  ObjKind kind_;
};
using ObjectPtr = std::shared_ptr<Object>;

//Int 类型
class IntObject: public Object {
public:
  IntObject(int value): Object(ObjKind::Int), value_(value) {}
public:
  int value_;
};
using IntObjectPtr = std::shared_ptr<IntObject>;

//Str 类型
class StrObject: public Object {
public:
  StrObject(const std::string &str): Object(ObjKind::String), str_(str) {}
public:
  std::string str_;
};
using StrObjectPtr = std::shared_ptr<StrObject>;

//BOOL 类型
class BoolObject: public Object {
public:
  BoolObject(bool b): Object(ObjKind::Bool), b_(b) {}
  BoolObject(int num): Object(ObjKind::Bool), b_(num != 0) {}
public:
  bool b_;
};
using BoolObjectPtr = std::shared_ptr<BoolObject>;

/******************************全局环境变量表****************************/

class Environment {
public:
  ObjectPtr get(const std::string &name) {
    try {
      return env_.at(name);
    }
    catch (std::out_of_range &e) {
      return nullptr;
    }
  }
  void put(const std::string &name, ObjectPtr obj) {
    env_[name] = obj;
  }

private:
  std::map<std::string, ObjectPtr> env_;
};

/******************************AST迭代器接口*****************************/
template <typename Item>
class Iterator {
public:
  virtual void first() = 0;
  virtual void next() = 0;
  virtual bool hasNext() const = 0;
  virtual Item current() const = 0;
};

/*****************************AST类型************************************/
enum class ASTKind {
  //用于标识自身身份,可用作元信息
  INVALID = -1,

  LEAF_COMMON = 1000, //普通叶子节点，如"{", "if", "while"这类
  LEAF_INT = 1001,
  LEAF_Id = 1002,
  LEAF_STR = 1003,

  LIST_COMMON = 2000, 
  LIST_PRIMARY_EXPR = 2001,
  LIST_NEGETIVE_EXPR = 2002,
  LIST_BINARY_EXPR = 2003,

  LIST_BLOCK_STMNT = 2004,
  LIST_IF_STMNT = 2005,
  LIST_WHILE_STMNT = 2006,
  LIST_NULL_STMNT = 2007,
};

/*********************AST的基类，用于定义接口****************************/
class ASTree;
using ASTreePtr = std::shared_ptr<ASTree>;

class ASTree {
public:
  ASTree(ASTKind astKind): kind_(astKind) {};

  //获取下标为i的子节点，
  virtual ASTreePtr child(int i) = 0;

  //返回子节点数量
  virtual int numChildren() = 0;

  //返回子节点集合的迭代器
  virtual Iterator<ASTreePtr> iterator() = 0;

  //获取该节点的信息
  virtual std::string info() = 0;

  //解析执行该节点并返回结果值，入参为环境变量
  virtual ObjectPtr eval(Environment &env) = 0;
public:
  ASTKind kind_;
};


/******************************AST相关的异常****************************/
class ASTException: public std::exception {
public:
  ASTException(const std::string &msg): errMsg_(msg) {}
  const char* what() const noexcept override {
    return errMsg_.c_str();
  }
private:
  std::string errMsg_;
};

class ASTEvalException: public ASTException {
public:
  ASTEvalException(const std::string &msg): ASTException(msg) {}
};

#endif
