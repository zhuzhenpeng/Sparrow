#ifndef SPARROW_AST_TREE_H_
#define SPARROW_AST_TREE_H_

#include <memory>
#include <string>
#include <exception>
#include "env.h"
#include "symbols.h"

#include "debugger.h"

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

//预处理异常
class ASTPreProcessException: public ASTException {
public:
  ASTPreProcessException(const std::string &msg): ASTException(msg) {}
};

//运行时异常
class ASTEvalException: public ASTException {
public:
  ASTEvalException(const std::string &msg): ASTException(msg) {}
};

//编译时异常
class ASTCompilingException: public ASTException {
public:
  ASTCompilingException(const std::string &msg): ASTException(msg) {}
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
  LIST_CONDITION_STMNT = 2005,
  LIST_AND_LOGIC = 2006,
  LIST_OR_LOGIC = 2007,
  LIST_IF_STMNT = 2008,
  LIST_ELIF_STMNT = 2009,
  LIST_WHILE_STMNT = 2010,
  LIST_NULL_STMNT = 2011,
  LIST_PARAMETER = 2012,
  LIST_DEF_STMNT = 2013,
  LIST_POSTFIX = 2014,
  LIST_ARGUMENTS = 2015,
  LIST_LAMB = 2016,
  LIST_CLASS_BODY = 2017,
  LIST_CLASS_STMNT = 2018,
  LIST_DOT = 2019,
  LIST_NEW = 2020,
  LIST_RETURN = 2021,
  LIST_ARRAY_LITERAL = 2022,
  LIST_ARRAY_REF = 2023,
  LIST_USING = 2024,
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

  //预处理，为某些节点生成所需的上下文信息，比如符号表
  virtual void preProcess(__attribute__((unused))SymbolsPtr symbols) {
    //默认情况下不需要进行任何处理
  }

  //编译生成机器代码
  virtual void compile() {
    //默认情况下不需要进行任何处理
  }

public:
  //AST类型
  ASTKind kind_;
};


#endif
