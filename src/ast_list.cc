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
  std::string result = "( ";
  for(auto child: children_) {
    result += child->info();
    result += " ";
  }
  result += " ";
  return result;
}

std::vector<ASTreePtr>& ASTList::children() {
  return children_;
}

/**************************元表达式****************************************/

PrimaryExprAST::PrimaryExprAST(): ASTList(ASTKind::LIST_PRIMARY_EXPR) {}


/**************************负值表达式*************************************/

NegativeExprAST::NegativeExprAST(): ASTList(ASTKind::LIST_NEGETIVE_EXPR) {}

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

/********************************块**************************************/

BlockStmntAST::BlockStmntAST(): ASTList(ASTKind::LIST_BLOCK_STMNT) {}

/******************************if块*************************************/

IfStmntAST::IfStmntAST(): ASTList(ASTKind::LIST_IF_STMNT) {}

ASTreePtr IfStmntAST::condition() {
  if (children_.empty())
    throw ASTException("get if AST condition failed");
  return children_[0];
}

ASTreePtr IfStmntAST::thenBlock() {
  if (children_.size() < 2)
    throw ASTException("get if AST then block failed");
  return children_[1];
}

ASTreePtr IfStmntAST::elseBlock() {
  if (children_.size() > 2)
    return children_[2];
  else
    return nullptr;
}

/****************************while块***********************************/

WhileStmntAST::WhileStmntAST(): ASTList(ASTKind::LIST_WHILE_STMNT) {}

ASTreePtr WhileStmntAST::condition() {
  if (children_.empty())
    throw ASTException("get while AST condition failed");
  return children_[0];
}

ASTreePtr WhileStmntAST::body() {
  if (children_.size() < 2)
    throw ASTException("get while AST body failed");
  return children_[1];
}

/****************************Null块************************************/

NullStmntAST::NullStmntAST(): ASTList(ASTKind::LIST_NULL_STMNT) {}
