#ifndef SPARROW_AST_LEAF_H_
#define SPARROW_AST_LEAF_H_

#include "ast_tree.h"
#include "token.h"

/*
 * AST叶节点，没有子节点
 */

class ASTLeaf: public ASTree {
public:
  ASTLeaf(TokenPtr token);

  //由于叶节点没有子节点了，所以总是返回空指针
  ASTreePtr child(int i) override;

  //总是返回0
  int numChildren() override;

  //抛出调用异常
  Iterator<ASTreePtr> children() override;

  std::string info() override;

  TokenPtr getToken() const;

protected:
  TokenPtr token_;
};
#endif
