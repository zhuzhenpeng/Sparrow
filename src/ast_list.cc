#include "ast_list.h"
#include "ast_leaf.h"
#include "debugger.h"

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
  std::string result = "(";
  MyDebugger::print(static_cast<int>(kind_), __FILE__, __LINE__);
  for (size_t i = 0; i < children_.size(); ++i) {
    result += children_[i]->info();
    if (i < children_.size() - 1)
      result += " ";
  }
  result += ")";
  return result;
}

ObjectPtr ASTList::eval(__attribute__((unused)) Environment &env) {
  throw ASTEvalException("call error, not evalable for AST list");
}

std::vector<ASTreePtr>& ASTList::children() {
  return children_;
}

/**************************元表达式****************************************/

PrimaryExprAST::PrimaryExprAST(): ASTList(ASTKind::LIST_PRIMARY_EXPR) {}

ObjectPtr PrimaryExprAST::eval(Environment &env) {
  return children_[0]->eval(env);
}

/**************************负值表达式*************************************/

NegativeExprAST::NegativeExprAST(): ASTList(ASTKind::LIST_NEGETIVE_EXPR) {}

std::string NegativeExprAST::info() {
  return "-" + children_[1]->info();
}

ObjectPtr NegativeExprAST::eval(Environment &env) {
  auto num = children_[1]->eval(env);
  if (num->kind_ == ObjKind::Int) {
    int positive = std::static_pointer_cast<IntObject>(num)->value_;
    return std::make_shared<IntObject>(-positive);
  }
  else {
    throw ASTEvalException("bad type for -");
  }
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
  auto id = std::dynamic_pointer_cast<ASTLeaf>(children_[1])->getToken();
  return id->getText();
}

ObjectPtr BinaryExprAST::eval(Environment &env) {
  std::string op = getOperator();
  if (op == "=") {
    ObjectPtr rightValue = rightFactor()->eval(env);
    return assignOp(env, rightValue);
  }
  else {
    ObjectPtr leftValue = leftFactor()->eval(env);
    ObjectPtr rightValue = rightFactor()->eval(env);
    return otherOp(leftValue, op, rightValue);
  }
}

ObjectPtr BinaryExprAST::assignOp(Environment &env, ObjectPtr rightValue) {
  auto leftTree = leftFactor();
  if (leftTree->kind_ == ASTKind::LEAF_Id) {
    env.put(std::dynamic_pointer_cast<IdTokenAST>(leftTree)->getId(), rightValue);
    return rightValue;
  }
  else {
    throw ASTEvalException("bad assign, not a valid id");
  }
}

ObjectPtr BinaryExprAST::otherOp(ObjectPtr left, const std::string &op, ObjectPtr right) {
  if (left->kind_ == ObjKind::Int && right->kind_ == ObjKind::Int) {
    return computeNumber(std::static_pointer_cast<IntObject>(left), op, 
        std::static_pointer_cast<IntObject>(right));
  } 
  else {
    auto strLeft = std::static_pointer_cast<StrObject>(left);
    auto strRight = std::static_pointer_cast<StrObject>(right);
    if (op == "+") {
      return std::make_shared<StrObject>(strLeft->str_ + strRight->str_);
    }
    else if (op == "==") {
      if (strLeft->str_ == strRight->str_)
        return std::make_shared<BoolObject>(true);
      else
        return std::make_shared<BoolObject>(false);
    }
    else {
      throw ASTEvalException("bad type for str operation");
    }
  }
}

ObjectPtr BinaryExprAST::computeNumber(IntObjectPtr left, const std::string &op, 
    IntObjectPtr right) {
  int a = left->value_;
  int b = right->value_;
  if (op == "+")
    return std::make_shared<IntObject>(a + b);
  else if (op == "-")
    return std::make_shared<IntObject>(a - b);
  else if (op == "*")
    return std::make_shared<IntObject>(a * b);
  else if (op == "/")
    return std::make_shared<IntObject>(a / b);
  else if (op == "%")
    return std::make_shared<IntObject>(a & b);
  else if (op == "==")
    return std::make_shared<BoolObject>(a == b);
  else if (op == ">")
    return std::make_shared<BoolObject>(a > b);
  else if (op == "<")
    return std::make_shared<BoolObject>(a < b);
  else
    throw ASTEvalException("bad operators for bianry expr");
}

void BinaryExprAST::checkValid() {
  if (children_.size() < 3) {
    throw ASTException("Binary expression AST is not valid, size: " + 
        std::to_string(children_.size()));
  }
}

/********************************块**************************************/

BlockStmntAST::BlockStmntAST(): ASTList(ASTKind::LIST_BLOCK_STMNT) {}

ObjectPtr BlockStmntAST::eval(Environment &env) {
  ObjectPtr result;
  for (auto subTree: children_) {
    if (!(subTree->kind_ == ASTKind::LIST_NULL_STMNT))
      result = subTree->eval(env);
  }
  return result;
}

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

std::string IfStmntAST::info() {
  std::string result = "(if ";
  result += condition()->info();
  result += " ";
  result += thenBlock()->info();
  auto elsePtr = elseBlock();
  if (elsePtr == nullptr) {
    result += ")";
  }
  else {
    result += " else ";
    result += elsePtr->info();
    result += ")";
  }
  return result;
}

ObjectPtr IfStmntAST::eval(Environment &env) {
  auto b = condition()->eval(env);
  if (b->kind_ != ObjKind::Bool)
    throw ASTEvalException("error type for if condition part");
  if (std::static_pointer_cast<BoolObject>(b)->b_) {
    return thenBlock()->eval(env);
  }
  else {
    auto eb = elseBlock();
    if (eb == nullptr)
      return nullptr;
    else
      return eb->eval(env);
  }
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

std::string WhileStmntAST::info() {
  std::string result = "(while ";
  result += condition()->info();
  result += " ";
  result += body()->info();
  result += ")";
  return result;
}

ObjectPtr WhileStmntAST::eval(Environment &env) {
  ObjectPtr result;
  while (true) {
    ObjectPtr con = condition()->eval(env);

    if (con->kind_ != ObjKind::Bool)
      throw ASTEvalException("error type for while condition part");
    if (!std::static_pointer_cast<BoolObject>(con)->b_) {
      return result;
    }
    else {
      result = body()->eval(env);
    }
  }
}

/****************************Null块************************************/

NullStmntAST::NullStmntAST(): ASTList(ASTKind::LIST_NULL_STMNT) {}

ObjectPtr NullStmntAST::eval(__attribute__((unused)) Environment &env) {
  return nullptr;
}
