#include "ast_leaf.h"

ASTLeaf::ASTLeaf(TokenPtr token): token_(token) {}

ASTreePtr ASTLeaf::child(__attribute__((unused)) int i) {
  return nullptr;
}

int ASTLeaf::numChildren() {
  return 0;
}

