#ifndef SPARROW_AST_LEAF_H_
#define SPARROW_AST_LEAF_H_

#include <vector>
#include "ast_tree.h"
#include "token.h"

/*
 * AST叶节点，没有子节点
 */

class ASTLeaf: public ASTree {
public:
  ASTLeaf(TokenPtr token);

  ASTreePtr child(int i) override;

  int numChildren() override;

  Itertor<ASTreePtr> children() override;

  std::string info() override;

protected:
  TokenPtr token_;

private:
  std::vector<ASTreePtr> unused_;
};

#endif
