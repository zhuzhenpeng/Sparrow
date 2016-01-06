#include "ast_list.h"
#include "ast_leaf.h"

/**************************AST内部（非叶子）节点******************************/

ASTList::ASTList(ASTKind kind): ASTree(kind) {}

ASTreePtr ASTList::child(int i) {
  return children_[i];
}

int ASTList::numChildren() {
  return children_.size();
}

Iterator<ASTreePtr> ASTList::iterator() {
  return ASTListIterator<ASTreePtr>(children_);
}

std::string ASTList::info() {
  std::string result; 
  for(auto child: children_) {
    result += child->info();
    result += "\n";
  }
  return result;
}

std::vector<ASTreePtr>& ASTList::children() {
  return children_;
}

/***********************二元表达式******************************************/

BinaryExprAST::BinaryExprAST(): ASTList(ASTKind::LIST_BINARY_EXPR) {}

ASTreePtr BinaryExprAST::leftFactor() {
  checkValid();
  return children_[0];
}

ASTreePtr BinaryExprAST::rightFactor() {
  checkValid();
  return children_[2];
}

std::string BinaryExprAST::getOperator() {
  checkValid();
  auto id = reinterpret_cast<ASTLeaf*>(children_[1].get())->getToken();
  return reinterpret_cast<IdToken*>(id.get())->getId();
}

void BinaryExprAST::checkValid() {
  if (children_.size() < 3) {
    throw ASTException("Binary expression AST is not valid, size: " + 
        std::to_string(children_.size()));
  }
}
