#include "ast_leaf.h"

/************************AST叶节点，没有子节点**************************/

ASTLeaf::ASTLeaf(ASTKind kind, TokenPtr token): ASTree(kind), token_(token) {}

ASTreePtr ASTLeaf::child(__attribute__((unused)) int i) {
  return nullptr;
}

int ASTLeaf::numChildren() {
  return 0;
}

Iterator<ASTreePtr> ASTLeaf::iterator() {
  throw ASTException("error call: no children for AST leaf");
}

std::string ASTLeaf::info() {
  return token_->getText();
}

ObjectPtr ASTLeaf::eval(__attribute__((unused)) EnvPtr env) {
  throw ASTEvalException("error call: not evalable for AST leaf");
}

TokenPtr ASTLeaf::getToken() const {
  return token_;
}

/*********************IntToken对应的叶子节点***************************/

IntTokenAST::IntTokenAST(TokenPtr token): ASTLeaf(ASTKind::LEAF_INT, token) {}

std::string IntTokenAST::info() {
  return std::to_string(getValue());
}

ObjectPtr IntTokenAST::eval(__attribute__((unused)) EnvPtr env) {
  return std::make_shared<IntObject>(getValue());
}

int IntTokenAST::getValue() const {
  return std::static_pointer_cast<IntToken>(token_)->getValue();
}

/*********************IdToken对应的叶子节点***************************/

IdTokenAST::IdTokenAST(TokenPtr token): ASTLeaf(ASTKind::LEAF_Id, token) {}

std::string IdTokenAST::info() {
  return getId();
}

ObjectPtr IdTokenAST::eval(EnvPtr env) {
  return env->get(getId()); 
}

std::string IdTokenAST::getId() const {
  return token_->getText();
}

/*********************StrToken对应的叶子节点*************************/

StrTokenAST::StrTokenAST(TokenPtr token): ASTLeaf(ASTKind::LEAF_STR, token) {}

std::string StrTokenAST::info() {
  return getContent();
}

ObjectPtr StrTokenAST::eval(__attribute__((unused)) EnvPtr env) {
  return std::make_shared<StrObject>(getContent());
}

std::string StrTokenAST::getContent() const {
  return token_->getText();
}
