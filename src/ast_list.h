#ifndef SPARROW_AST_LEAF_H_
#define SPARROW_AST_LEAF_H_

#include "ast_tree.h"
#include <vector>

class ASTList: public ASTree {
public:
  ASTreePtr child(int i) override;

  int numChildren() override;

  Iterator<ASTreePtr> children() override;

  std::string info() override;

private:
  std::vector<ASTreePtr> children_();
};

#endif
