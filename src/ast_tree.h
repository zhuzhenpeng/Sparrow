#ifndef SPARROW_AST_TREE_H_
#define SPARROW_AST_TREE_H_

#include <memory>
#include <string>
#include <exception>

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
  LIST_BINARY_EXPR = 2001,
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


#endif
