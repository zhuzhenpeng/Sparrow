#include "ast_leaf.h"

/************************AST叶节点，没有子节点**************************/

ASTLeaf::ASTLeaf(ASTKind kind, TokenPtr token): ASTree(kind), token_(token) {}

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

IntTokenAST::IntTokenAST(TokenPtr token): ASTLeaf(ASTKind::LEAF_INT, token) {}

int IntTokenAST::getValue() const {
  return reinterpret_cast<IntToken*>(token_.get())->getValue();
}

/*********************IdToken对应的叶子节点***************************/

IdTokenAST::IdTokenAST(TokenPtr token): ASTLeaf(ASTKind::LEAF_Id, token) {}

std::string IdTokenAST::getId() const {
  return reinterpret_cast<IdToken*>(token_.get())->getId();
}

/*********************StrToken对应的叶子节点*************************/

StrTokenAST::StrTokenAST(TokenPtr token): ASTLeaf(ASTKind::LEAF_STR, token) {}

std::string StrTokenAST::getContent() const {
  return reinterpret_cast<StrToken*>(token_.get())->getString();
}
