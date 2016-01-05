#include "ast_leaf.h"

/************************AST叶节点，没有子节点**************************/

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

/*********************IntToken对应的叶子节点***************************/

IntTokenAST::IntTokenAST(TokenPtr token): ASTLeaf(token) {
  kind_ = ASTKind::LEAF_INT;
}

int IntTokenAST::getValue() const {
  return reinterpret_cast<IntToken*>(token_.get())->getValue();
}

/*********************IdToken对应的叶子节点***************************/

IdTokenAST::IdTokenAST(TokenPtr token): ASTLeaf(token) {
  kind_ = ASTKind::LEAF_Id;
}

std::string IdTokenAST::getId() const {
  return reinterpret_cast<IdToken*>(token_.get())->getId();
}
