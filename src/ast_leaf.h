#ifndef SPARROW_AST_LEAF_H_
#define SPARROW_AST_LEAF_H_

#include "ast_tree.h"
#include "token.h"

/************************AST叶节点，没有子节点**************************/

class ASTLeaf: public ASTree {
public:
  ASTLeaf(ASTKind kind, TokenPtr token);

  //由于叶节点没有子节点了，所以总是返回空指针
  ASTreePtr child(int i) override;

  //总是返回0
  int numChildren() override;

  //抛出调用异常
  Iterator<ASTreePtr> iterator() override;

  std::string info() override;

  //抛出调用异常
  ObjectPtr eval(EnvPtr env) override;
  
  //抛出调用异常
  //void preProcess(__attribute__((unused))SymbolsPtr symbols) override;

  TokenPtr getToken() const;

  void setIndex(size_t index);

  size_t getIndex() const;

protected:
  TokenPtr token_;
  
  //在符号表中的下标
  //整型、浮点数、字符串分别对应三个常量池
  //Id所指向的变量如果是局部变量，则是局部变量槽的下标，否则没有意义
  int index_ = -1;
};

/*********************IntToken对应的叶子节点***************************/

class IntTokenAST: public ASTLeaf {
public:
  IntTokenAST(TokenPtr token); 
  std::string info() override;
  ObjectPtr eval(EnvPtr env) override;
  int getValue() const;

  //把数值填入常量池，并记录下相应的坐标
  //不需要用到入参符号表，而是全局整型数常量符号表
  void preProcess(__attribute__((unused))SymbolsPtr symbols) override;
};

/*********************FloatToken对应的叶子节点************************/

class FloatTokenAST: public ASTLeaf {
public:
  FloatTokenAST(TokenPtr token);
  std::string info() override;
  ObjectPtr eval(EnvPtr env) override;
  double getValue() const;
  
  //把数值填入常量池，并记录下相应的坐标
  //不需要用到入参符号表，而是的全局浮点数常量符号表
  void preProcess(__attribute__((unused))SymbolsPtr symbols) override;
};

/*********************IdToken对应的叶子节点***************************/

//ID的三种类型：全局、局部、闭包中引用的外层函数变量
enum class IdKind {
  UNKNOWN, GLOBAL, LOCAL, CLOSURE
};

class IdTokenAST: public ASTLeaf {
public:
  IdTokenAST(TokenPtr token);
  std::string info() override;
  ObjectPtr eval(EnvPtr env) override;
  std::string getId() const;

  //赋值操作
  void assign(EnvPtr env, ObjectPtr value);

  //检查该id指向变量的性质：全局变量、局部变量、闭包引用的外部函数变量
  //设置变量的属性
  //如果是局部变量，记录下该变量在运行时环境的位置信息
  void preProcess(SymbolsPtr symbols) override;

  IdKind kind_ = IdKind::UNKNOWN;
};

/*********************StrToken对应的叶子节点*************************/
class StrTokenAST: public ASTLeaf {
public:
  StrTokenAST(TokenPtr token);
  std::string info() override;
  ObjectPtr eval(EnvPtr env) override;
  std::string getContent() const;
  
  //把字符串填入常量池，并记录下相应的坐标
  //不需要用到入参符号表，而是全局字符串常量符号表
  void preProcess(__attribute__((unused))SymbolsPtr symbols) override;
};

#endif
