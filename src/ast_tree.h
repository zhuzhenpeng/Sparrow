#ifndef SPARROW_AST_TREE_H_
#define SPARROW_AST_TREE_H_

#include <memory>
#include <string>
#include <exception>
#include "env.h"

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

/******************************AST迭代器接口*****************************/
template <typename Item>
class Iterator {
public:
  virtual void first() {
    throw ASTException("un-implement Iterator exception!");
  }
  virtual void next() {
    throw ASTException("un-implement Iterator exception!");
  }
  virtual bool hasNext() const {
    throw ASTException("un-implement Iterator exception!");
  }
  virtual Item current() const {
    throw ASTException("un-implement Iterator exception!");
  }
};

/*****************************AST类型************************************/
enum class ASTKind {
  //用于标识自身身份,可用作元信息
  INVALID = -1,

  LEAF_COMMON = 1000, //普通叶子节点，如"{", "if", "while"这类
  LEAF_INT = 1001,
  LEAF_FLOAT = 1002,
  LEAF_Id = 1003,
  LEAF_STR = 1004,

  LIST_COMMON = 2000, 
  LIST_PRIMARY_EXPR = 2001,
  LIST_NEGETIVE_EXPR = 2002,
  LIST_BINARY_EXPR = 2003,

  LIST_BLOCK_STMNT = 2004,
  LIST_IF_STMNT = 2005,
  LIST_WHILE_STMNT = 2006,
  LIST_NULL_STMNT = 2007,
  LIST_PARAMETER = 2008,
  LIST_DEF_STMNT = 2009,
  LIST_POSTFIX = 2010,
  LIST_ARGUMENTS = 2011,
  LIST_LAMB = 2012,
  LIST_CLASS_BODY = 2013,
  LIST_CLASS_STMNT = 2014,
  LIST_DOT = 2015,
  LIST_ARRAY_LITERAL = 2016,
  LIST_ARRAY_REF = 2017 
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
  virtual ObjectPtr eval(EnvPtr env) = 0;

public:
  //AST类型
  ASTKind kind_;
};


#endif
