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

  TokenPtr getToken() const;

protected:
  TokenPtr token_;
};

/*********************IntToken对应的叶子节点***************************/

class IntTokenAST: public ASTLeaf {
public:
  IntTokenAST(TokenPtr token); 
  std::string info() override;
  ObjectPtr eval(EnvPtr env) override;
  int getValue() const;
};

/*********************IdToken对应的叶子节点***************************/

class IdTokenAST: public ASTLeaf {
public:
  IdTokenAST(TokenPtr token);
  std::string info() override;
  ObjectPtr eval(EnvPtr env) override;
  std::string getId() const;
};

/*********************StrToken对应的叶子节点*************************/
class StrTokenAST: public ASTLeaf {
public:
  StrTokenAST(TokenPtr token);
  std::string info() override;
  ObjectPtr eval(EnvPtr env) override;
  std::string getContent() const;
};

#endif
