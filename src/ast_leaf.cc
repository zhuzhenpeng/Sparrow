#include "ast_leaf.h"

ASTLeaf::ASTLeaf(TokenPtr token): token_(token) {}

ASTreePtr ASTLeaf::child(__attribute__((unused)) int i) {
  return nullptr;
}

int ASTLeaf::numChildren() {
  return 0;
}

Iterator<ASTreePtr> ASTLeaf::children() {
  throw ASTException("error call: no children for AST leaf");
}

std::string ASTLeaf::info() {
  std::string result = std::string("file: ") + token_->getFileName() + 
    ", line: " + std::to_string(token_->getLineNumber()) +
    ", raw text: " + token_->getText();
  return result;
}

TokenPtr ASTLeaf::getToken() const {
  return token_;
}
