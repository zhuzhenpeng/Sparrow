#include "ast_list.h"
#include "ast_leaf.h"
#include "debugger.h"

#include <cmath>

/**************************AST内部（非叶子）节点******************************/

ASTList::ASTList(ASTKind kind, bool ignore): ASTree(kind), ignore_(ignore) {}

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

  //std::string size = std::to_string(children_.size());
  //result += "[" + size + "]";
  
  //std::string ignore = std::to_string(ignore_);
  //result += "[" + ignore + "]";
  
  //std::string kind = std::to_string(static_cast<int>(kind_));
  //result += "[" + kind + "]";

  for (size_t i = 0; i < children_.size(); ++i) {
    result += children_[i]->info();
    if (i < children_.size() - 1)
      result += " ";
  }
  result += ")";
  return result;
}

ObjectPtr ASTList::eval(__attribute__((unused)) EnvPtr env) {
  throw ASTEvalException("call error, not evalable for AST list");
}

void ASTList::preProcess(SymbolsPtr symbols) {
  for (auto &child: children_)
    child->preProcess(symbols);
}

std::vector<ASTreePtr>& ASTList::children() {
  return children_;
}

bool ASTList::ignore() const {
  return ignore_;
}

/**************************元表达式****************************************/

PrimaryExprAST::PrimaryExprAST(): ASTList(ASTKind::LIST_PRIMARY_EXPR, true) {}

ObjectPtr PrimaryExprAST::eval(EnvPtr env) {
  return evalSubExpr(env, 0); 
}

ASTreePtr PrimaryExprAST::operand() {
  if (children_.empty())
    throw ASTException("no children for Primary AST, can't get operand");
  return children_[0];
}

std::shared_ptr<PostfixAST> PrimaryExprAST::postfix(size_t nest) {
  return std::static_pointer_cast<PostfixAST>(children_[children_.size() - nest - 1]);
}

bool PrimaryExprAST::hasPostfix(size_t nest) {
  return children_.size() > (nest + 1);
}

ObjectPtr PrimaryExprAST::evalSubExpr(EnvPtr env, size_t nest) {
  ObjectPtr result = nullptr;
  if (hasPostfix(nest)) {
    //如果存在后缀则递归地求值
    //caller往往是左侧求出来的函数对象、数组对象
    ObjectPtr caller = evalSubExpr(env, nest + 1);
    result = postfix(nest)->eval(env, caller);
  }
  else {
    //MyDebugger::print("--", __FILE__, __LINE__);
    result = operand()->eval(env);
  }
  return result;
}

/**************************负值表达式*************************************/

NegativeExprAST::NegativeExprAST(): ASTList(ASTKind::LIST_NEGETIVE_EXPR, false) {}

std::string NegativeExprAST::info() {
  return "-" + children_[1]->info();
}

ObjectPtr NegativeExprAST::eval(EnvPtr env) {
  auto num = children_[1]->eval(env);
  if (num->kind_ == ObjKind::INT) {
    int positive = std::static_pointer_cast<IntObject>(num)->value_;
    return std::make_shared<IntObject>(-positive);
  }
  else if (num->kind_ == ObjKind::FLOAT) {
    double positive = std::static_pointer_cast<FloatObject>(num)->value_;
    return std::make_shared<FloatObject>(-positive);
  }
  else {
    throw ASTEvalException("bad type for -");
  }
}

/***********************二元表达式******************************************/

BinaryExprAST::BinaryExprAST(): ASTList(ASTKind::LIST_BINARY_EXPR, false) {}

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

ObjectPtr BinaryExprAST::eval(EnvPtr env) {
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

ObjectPtr BinaryExprAST::assignOp(EnvPtr env, ObjectPtr rightValue) {
  auto leftTree = leftFactor();
  if (leftTree->kind_ == ASTKind::LEAF_Id) {
    auto idToken = std::dynamic_pointer_cast<IdTokenAST>(leftTree);
    idToken->assign(env, rightValue);
    return rightValue;
  }
  else if (leftTree->kind_ == ASTKind::LIST_PRIMARY_EXPR) {
    PrimaryExprPtr primary = std::dynamic_pointer_cast<PrimaryExprAST>(leftTree);

    //.域访问
    if (primary->hasPostfix(0) && primary->postfix(0)->kind_ == ASTKind::LIST_DOT) {
      ObjectPtr obj = primary->evalSubExpr(env, 1);
      if (obj->kind_ == ObjKind::CLASS_INSTANCE) {
        auto dot = std::dynamic_pointer_cast<Dot>(primary->postfix(0));
        return setInstanceField(std::dynamic_pointer_cast<ClassInstance>(obj), 
            dot->name(), rightValue);
      }
      else if (obj->kind_ == ObjKind::ENV) {
        EnvPtr targetEnv = std::dynamic_pointer_cast<CommonEnv>(obj);
        auto dot = std::dynamic_pointer_cast<Dot>(primary->postfix(0));
        targetEnv->put(dot->name(), rightValue);
        return rightValue;
      }
      else {
        throw ASTEvalException("bad assign, left value is not a valid class instance"\
            " or environment alias");
      }
    }
    //数组访问
    else if (primary->hasPostfix(0) && primary->postfix(0)->kind_ == ASTKind::LIST_ARRAY_REF) {
      ObjectPtr array = primary->evalSubExpr(env, 1);
      if (array->kind_ == ObjKind::Array) {
        ArrayRefPtr arrRef = std::dynamic_pointer_cast<ArrayRefAST>(primary->postfix(0));
        ObjectPtr index = arrRef->index()->eval(env);
        if (index->kind_ == ObjKind::INT) {
          auto a = std::dynamic_pointer_cast<Array>(array);
          auto i = std::dynamic_pointer_cast<IntObject>(index);
          a->set(i->value_, rightValue);
          return rightValue;
        }
        else {
          throw ASTEvalException("not a valid int type for array access");
        }
      }
      else {
        throw ASTEvalException("bad assign, left value is not a valid array");
      }
    }
    else {
      throw ASTEvalException("bad assign, not a valid primary expression");
    }
  }
  else {
    throw ASTEvalException("bad assign, not a valid left value");
  }
}

ObjectPtr BinaryExprAST::otherOp(ObjectPtr left, const std::string &op, ObjectPtr right) {
  if (left->kind_ == ObjKind::INT && right->kind_ == ObjKind::INT) {
    int leftInt = std::static_pointer_cast<IntObject>(left)->value_;
    int rightInt = std::static_pointer_cast<IntObject>(right)->value_;
    return computeInt(leftInt, op, rightInt);
  } 
  else if (left->kind_ == ObjKind::FLOAT && right->kind_ == ObjKind::FLOAT) {
    double leftFloat = std::static_pointer_cast<FloatObject>(left)->value_;
    double rightFloat = std::static_pointer_cast<FloatObject>(right)->value_;
    return computeFloat(leftFloat, op, rightFloat);
  }
  else if (left->kind_ == ObjKind::INT && right->kind_ == ObjKind::FLOAT) {
    double leftFloat = std::static_pointer_cast<IntObject>(left)->value_;
    double rightFloat = std::static_pointer_cast<FloatObject>(right)->value_;
    return computeFloat(leftFloat, op, rightFloat);
  }
  else if (left->kind_ == ObjKind::FLOAT && right->kind_ == ObjKind::INT) {
    double leftFloat = std::static_pointer_cast<FloatObject>(left)->value_;
    double rightFloat = std::static_pointer_cast<IntObject>(right)->value_;
    return computeFloat(leftFloat, op, rightFloat);
  }
  else if (left->kind_ == ObjKind::STRING && right->kind_ == ObjKind::STRING){
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
    else if (op == "!=") {
      if (strLeft->str_ != strRight->str_)
        return std::make_shared<BoolObject>(true);
      else
        return std::make_shared<BoolObject>(false);
    }
    else {
      throw ASTEvalException("bad type for str operation");
    }
  }
  else {
    throw ASTEvalException("invalid type for binary operation");
  }
}

ObjectPtr BinaryExprAST::computeInt(int left, const std::string &op, int right) {
  if (op == "+")
    return std::make_shared<IntObject>(left + right);
  else if (op == "-")
    return std::make_shared<IntObject>(left - right);
  else if (op == "*")
    return std::make_shared<IntObject>(left * right);
  else if (op == "/")
    return std::make_shared<IntObject>(left / right);
  else if (op == "%")
    return std::make_shared<IntObject>(left % right);
  else if (op == "==")
    return std::make_shared<BoolObject>(left == right);
  else if (op == "!=")
    return std::make_shared<BoolObject>(left != right);
  else if (op == ">=")
    return std::make_shared<BoolObject>(left >= right);
  else if (op == "<=")
    return std::make_shared<BoolObject>(left <= right);
  else if (op == ">")
    return std::make_shared<BoolObject>(left > right);
  else if (op == "<")
    return std::make_shared<BoolObject>(left < right);
  else
    throw ASTEvalException("bad operators for bianry expr");
}

ObjectPtr BinaryExprAST::computeFloat(double left, const std::string &op, double right) {
  //%运算符属于非法运算符
  double diff = std::abs(left - right);
  if (op == "+")
    return std::make_shared<FloatObject>(left + right);
  else if (op == "-")
    return std::make_shared<FloatObject>(left - right);
  else if (op == "*")
    return std::make_shared<FloatObject>(left * right);
  else if (op == "/")
    return std::make_shared<FloatObject>(left / right);
  else if (op == "==")
    return std::make_shared<BoolObject>(diff <= 1e-10);
  else if (op == "!=")
    return std::make_shared<BoolObject>(diff > 1e-10);
  else if (op == ">=")
    return std::make_shared<BoolObject>(left >= right);
  else if (op == "<=")
    return std::make_shared<BoolObject>(left <= right);
  else if (op == ">")
    return std::make_shared<BoolObject>(left > right);
  else if (op == "<")
    return std::make_shared<BoolObject>(left < right);
  else
    throw ASTEvalException("bad operators for bianry expr: " + op);
}

ObjectPtr BinaryExprAST::setInstanceField(InstancePtr obj, const std::string &filedName, 
    ObjectPtr rvalue) {
  obj->write(filedName, rvalue);  
  return obj;
}

void BinaryExprAST::checkValid() {
  if (children_.size() < 3) {
    throw ASTException("Binary expression AST is not valid, size: " + 
        std::to_string(children_.size()));
  }
}

/*****************************条件判断**********************************/

ConditionStmntAST::ConditionStmntAST(): ASTList(ASTKind::LIST_CONDITION_STMNT, false) {}

std::string ConditionStmntAST::info() {
  if (children_.size() != 1)
    throw ASTEvalException("fatal error, error size for condition AST");
  return children_[0]->info();
}

ObjectPtr ConditionStmntAST::eval(EnvPtr env) {
  if (children_.size() != 1)
    throw ASTEvalException("fatal error, error size for condition AST");
  return children_[0]->eval(env);
}

/******************************与逻辑***********************************/

AndLogicAST::AndLogicAST(): ASTList(ASTKind::LIST_AND_LOGIC, false) {}

ConditionStmntPtr AndLogicAST::leftExpr() {
  if (children_.size() < 1)
    throw ASTEvalException("fatal error, not found left conditon for and logic");
  if (children_[0]->kind_ != ASTKind::LIST_CONDITION_STMNT)
    throw ASTEvalException("error AST kind for left expression");
  return std::dynamic_pointer_cast<ConditionStmntAST>(children_[0]);
}

ConditionStmntPtr AndLogicAST::rightExpr() {
  if (children_.size() < 2)
    throw ASTEvalException("fatal error, not found right conditon for and logic");
  if (children_[0]->kind_ != ASTKind::LIST_CONDITION_STMNT)
    throw ASTEvalException("error AST kind for right expression");
  return std::dynamic_pointer_cast<ConditionStmntAST>(children_[1]);
}

std::string AndLogicAST::info() {
  std::string result = "(and ";
  result += leftExpr()->info();
  result += " ";
  result += rightExpr()->info();
  result += ")";
  return result;
}

ObjectPtr AndLogicAST::eval(EnvPtr env) {
  ObjectPtr leftResult = leftExpr()->eval(env);
  if (leftResult->kind_ != ObjKind::BOOL)
    throw ASTEvalException("error type for predicate: left factor in and logic");
  ObjectPtr rightResult = rightExpr()->eval(env);
  if (rightResult->kind_ != ObjKind::BOOL)
    throw ASTEvalException("error type for predicate: right factor in and logic");
  bool result = std::dynamic_pointer_cast<BoolObject>(leftResult)->b_ &&\
                std::dynamic_pointer_cast<BoolObject>(rightResult)->b_;
  return std::make_shared<BoolObject>(result);
}

/*****************************或逻辑************************************/

OrLogicAST::OrLogicAST(): ASTList(ASTKind::LIST_OR_LOGIC, false) {}

ConditionStmntPtr OrLogicAST::leftExpr() {
  if (children_.size() < 1)
    throw ASTEvalException("fatal error, not found left conditon for or logic");
  if (children_[0]->kind_ != ASTKind::LIST_CONDITION_STMNT)
    throw ASTEvalException("error AST kind for left expression");
  return std::dynamic_pointer_cast<ConditionStmntAST>(children_[0]);
}

ConditionStmntPtr OrLogicAST::rightExpr() {
  if (children_.size() < 2)
    throw ASTEvalException("fatal error, not found right conditon for or logic");
  if (children_[0]->kind_ != ASTKind::LIST_CONDITION_STMNT)
    throw ASTEvalException("error AST kind for right expression");
  return std::dynamic_pointer_cast<ConditionStmntAST>(children_[1]);
}

std::string OrLogicAST::info() {
  std::string result = "(or ";
  result += leftExpr()->info();
  result += " ";
  result += rightExpr()->info();
  result += ")";
  return result;
}

ObjectPtr OrLogicAST::eval(EnvPtr env) {
  ObjectPtr leftResult = leftExpr()->eval(env);
  if (leftResult->kind_ != ObjKind::BOOL)
    throw ASTEvalException("error type for predicate: left factor in or logic");
  ObjectPtr rightResult = rightExpr()->eval(env);
  if (rightResult->kind_ != ObjKind::BOOL)
    throw ASTEvalException("error type for predicate: right factor in or logic");
  bool result = std::dynamic_pointer_cast<BoolObject>(leftResult)->b_ ||\
                std::dynamic_pointer_cast<BoolObject>(rightResult)->b_;
  return std::make_shared<BoolObject>(result);
}


/********************************块**************************************/

BlockStmntAST::BlockStmntAST(): ASTList(ASTKind::LIST_BLOCK_STMNT, false) {}

ObjectPtr BlockStmntAST::eval(EnvPtr env) {
  ObjectPtr result;
  for (auto subTree: children_) {
      result = subTree->eval(env);
  }
  return result;
}

/******************************if块*************************************/

IfStmntAST::IfStmntAST(): ASTList(ASTKind::LIST_IF_STMNT, false) {}

ConditionStmntPtr IfStmntAST::condition() {
  if (children_.empty())
    throw ASTException("no sub AST in if AST");
  if (children_[0]->kind_ != ASTKind::LIST_CONDITION_STMNT)
    throw ASTException("error AST kind for if condition");
  return std::dynamic_pointer_cast<ConditionStmntAST>(children_[0]);
}

ASTreePtr IfStmntAST::thenBlock() {
  if (children_.size() < 2)
    throw ASTException("get if AST then block failed");
  return children_[1];
}

ASTreePtr IfStmntAST::elseBlock() {
  if (children_.size() > 2 && children_.back()->kind_ != ASTKind::LIST_ELIF_STMNT)
    return children_.back();
  else
    return nullptr;
}

std::string IfStmntAST::info() {
  std::string result = "(if ";
  result += condition()->info();
  result += " ";
  result += thenBlock()->info();

  for (size_t i = 2; i < children_.size(); ++i) {
    if (children_[i]->kind_ == ASTKind::LIST_ELIF_STMNT) {
      result += " ";
      result += children_[i]->info();
    }
  }

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

ObjectPtr IfStmntAST::eval(EnvPtr env) {
  auto ifCond = condition()->eval(env);
  if (ifCond->kind_ != ObjKind::BOOL)
    throw ASTEvalException("error type for if condition part");
  if (std::static_pointer_cast<BoolObject>(ifCond)->b_) {
    return thenBlock()->eval(env);
  }
  else if (children_.size() >= 3 && children_[2]->kind_ == ASTKind::LIST_ELIF_STMNT) {
    for (size_t i = 2; i < children_.size(); ++i) {
      //避免执行了else逻辑，每次遍历要判断下AST类型
      if (children_[i]->kind_ == ASTKind::LIST_ELIF_STMNT) {
        auto result = children_[i]->eval(env);
        if (result != nullptr)
          return result;
      }
      else {
        break;
      }
    }
  }

  //如果解释器运行至此，说明源程序的if和elif谓言为假或没有elif块，此时执行else块
  auto eb = elseBlock();
  if (eb == nullptr)
    return nullptr;
  else
    return eb->eval(env);
}

/******************************elif块**********************************/

ElifStmntAST::ElifStmntAST(): ASTList(ASTKind::LIST_ELIF_STMNT, false) {}

ConditionStmntPtr ElifStmntAST::condition() {
  if (children_.empty())
    throw ASTException("no sub AST in elif AST");
  if (children_[0]->kind_ != ASTKind::LIST_CONDITION_STMNT)
    throw ASTException("error AST type for elif condition");
  return std::dynamic_pointer_cast<ConditionStmntAST>(children_[0]);
}

ASTreePtr ElifStmntAST::thenBlock() {
  if (children_.size() < 2)
    throw ASTException("not found then block in elif AST");
  return children_[1];
}

std::string ElifStmntAST::info() {
  std::string result = "(elif ";
  result += condition()->info();
  result += " ";
  result += thenBlock()->info();
  result += ")";
  return result;
}

ObjectPtr ElifStmntAST::eval(EnvPtr env) {
  ObjectPtr condResult = condition()->eval(env);
  if (condResult->kind_ != ObjKind::BOOL)
    throw ASTEvalException("error type for elif predicate");
  if (std::dynamic_pointer_cast<BoolObject>(condResult)->b_) {
    return thenBlock()->eval(env);
  }
  else {
    return nullptr;
  }
}

/****************************while块***********************************/

WhileStmntAST::WhileStmntAST(): ASTList(ASTKind::LIST_WHILE_STMNT, false) {}

ConditionStmntPtr WhileStmntAST::condition() {
  if (children_.empty())
    throw ASTException("no sub AST in while AST");
  if (children_[0]->kind_ != ASTKind::LIST_CONDITION_STMNT)
    throw ASTException("error AST kind for while condition");
  return std::dynamic_pointer_cast<ConditionStmntAST>(children_[0]);
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

ObjectPtr WhileStmntAST::eval(EnvPtr env) {
  ObjectPtr result;
  while (true) {
    ObjectPtr con = condition()->eval(env);

    if (con->kind_ != ObjKind::BOOL)
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

NullStmntAST::NullStmntAST(): ASTList(ASTKind::LIST_NULL_STMNT, true) {}

ObjectPtr NullStmntAST::eval(__attribute__((unused)) EnvPtr env) {
  //NULL块的子节点总是为空，且可被忽略，因此不会出现在AST中
  throw ASTEvalException("Null Stmnt AST should not appear in AST, fatal error");
}

/******************************形参列表*******************************/

ParameterListAST::ParameterListAST(): ASTList(ASTKind::LIST_PARAMETER, false) {}

std::string ParameterListAST::paramName(size_t i) {
  return std::static_pointer_cast<ASTLeaf>(children_[i])->getToken()->getText();
}

size_t ParameterListAST::size() const {
  return children_.size();
}

void ParameterListAST::preProcess(SymbolsPtr symbols) {
  paramsOffset_.clear();
  for(size_t i = 0; i < size(); ++i)
    paramsOffset_.push_back(symbols->getRuntimeIndex(paramName(i)));
  //MyDebugger::print(paramsOffset_.size(), __FILE__, __LINE__);
}

void ParameterListAST::eval(EnvPtr funcEnv, EnvPtr callerEnv, 
    const std::vector<ASTreePtr> &args) {
  //MyDebugger::print(paramsOffset_.size(), __FILE__, __LINE__);
  for (size_t i = 0; i < args.size(); ++i) {
    funcEnv->put(paramsOffset_[i], args[i]->eval(callerEnv));
  }
}

/**************************函数定义块********************************/

DefStmntAST::DefStmntAST(): ASTList(ASTKind::LIST_DEF_STMNT, false) {}

std::string DefStmntAST::funcName() {
  return std::static_pointer_cast<ASTLeaf>(children_[0])->getToken()->getText();
}

ParameterListPtr DefStmntAST::parameterList() {
  return std::static_pointer_cast<ParameterListAST>(children_[1]);
}

BlockStmntPtr DefStmntAST::block() {
  return std::static_pointer_cast<BlockStmntAST>(children_[2]);
}

std::string DefStmntAST::info() {
  return std::string("(def ") + funcName() + " " + children_[1]->info() + " " +\
    children_[2]->info() + ")";
}

void DefStmntAST::preProcess(SymbolsPtr symbols) {
  symbols->getRuntimeIndex(funcName());
  localVarSize_ = getLocalVarSize(symbols, parameterList(), block());
}

ObjectPtr DefStmntAST::eval(EnvPtr env) {
  auto funcObj = std::make_shared<FuncObject>(funcName(), localVarSize_,
                                parameterList(), block(), env);
  env->put(funcName(), funcObj);
  return nullptr;
}

size_t DefStmntAST::getLocalVarSize(SymbolsPtr outer, 
    ParameterListPtr params, BlockStmntPtr block) {
  //运行时符号表
  SymbolsPtr runTimeSymbols = std::make_shared<Symbols>(outer, true);
  params->preProcess(runTimeSymbols);
  block->preProcess(runTimeSymbols);
  return runTimeSymbols->getSymbolSize();
}

/***************************实参***********************************/

ArgumentsAST::ArgumentsAST(): PostfixAST(ASTKind::LIST_ARGUMENTS, false) {}

size_t ArgumentsAST::size() const {
  return children_.size();
}

ObjectPtr ArgumentsAST::eval(EnvPtr env, ObjectPtr caller) {
  if (caller->kind_ != ObjKind::FUNCTION && caller->kind_ != ObjKind::NATIVE_FUNC)
    throw ASTEvalException("bad function for eval");

  if (caller->kind_ == ObjKind::NATIVE_FUNC) {
    return invokeNative(env, std::dynamic_pointer_cast<NativeFunction>(caller));
  }

  auto func = std::static_pointer_cast<FuncObject>(caller);
  auto params = func->params();
  if (size() != params->size()){
    MyDebugger::print(std::to_string(params->size()), __FILE__, __LINE__);
    throw ASTEvalException("error function call, not match the number of parameters");
  }

  //每次调用函数，都有创建一个新的环境，数组环境
  EnvPtr funcEnv = func->runtimeEnv();
  params->eval(funcEnv, env, children_);
  try {
    func->block()->eval(funcEnv);
  }
  catch(ASTReturnException &e) {
    return e.result_;
  }
  return nullptr;
}

ObjectPtr ArgumentsAST::invokeNative(EnvPtr env, NativeFuncPtr func) {
  if (size() != func->paramNum())
    throw ASTEvalException("error function call, not match the number of parameters");

  std::vector<ObjectPtr> params;
  for (auto t: children_)
    params.push_back(t->eval(env));

  return func->invoke(params);
}

/**************************闭包**********************************/

LambAST::LambAST(): ASTList(ASTKind::LIST_LAMB, false) {}

ParameterListPtr LambAST::parameterList() {
  return std::dynamic_pointer_cast<ParameterListAST>(children_[0]);
}

BlockStmntPtr LambAST::block() {
  return std::dynamic_pointer_cast<BlockStmntAST>(children_[1]);
}

std::string LambAST::info() {
  return "(lamb " + parameterList()->info() + " " + block()->info() + ")";
}

void LambAST::preProcess(SymbolsPtr symbols) {
  localVarSize_ = DefStmntAST::getLocalVarSize(symbols, parameterList(), 
      block());
}

ObjectPtr LambAST::eval(EnvPtr env) {
  //lambda创建的闭包都用CLOSURE来表示它的函数名
  return std::make_shared<FuncObject>("CLOSURE", localVarSize_,
      parameterList(), block(), env);
}

/************************类************************************/

ClassBodyAST::ClassBodyAST(): ASTList(ASTKind::LIST_CLASS_BODY, false) {}

ObjectPtr ClassBodyAST::eval(EnvPtr env) {
  for(auto child: children_)
    child->eval(env);
  return nullptr;
}

////ClassStmnt

ClassStmntAST::ClassStmntAST(): ASTList(ASTKind::LIST_CLASS_STMNT, false) {}

std::string ClassStmntAST::name() {
  auto nameLeaf = std::dynamic_pointer_cast<ASTLeaf>(children_[0]);
  if (nameLeaf == nullptr)
    throw ASTException("get class name failed, unknown type for first child");
  return nameLeaf->getToken()->getText();
}

std::string ClassStmntAST::superClassName() {
  if (numChildren() < 3) {
    return "";
  }
  else {
    auto nameLeaf = std::dynamic_pointer_cast<ASTLeaf>(children_[1]);
    if (nameLeaf == nullptr)
      throw ASTException("get super class name failed");
    return nameLeaf->getToken()->getText(); 
  }
}

ClassBodyPtr ClassStmntAST::body() {
  auto body = std::dynamic_pointer_cast<ClassBodyAST>(children_.back());
  if (body == nullptr)
    throw ASTEvalException("get class body failed");
  return body;
}

void ClassStmntAST::preProcess(SymbolsPtr symbols) {
  //auto result = symbols->getRuntimeIndex(name());
  //MyDebugger::print(result, __FILE__, __LINE__);
  symbols->getRuntimeIndex("super");
  SymbolsPtr classSymbols = std::make_shared<Symbols>(symbols, false);
  body()->preProcess(classSymbols);
}

ObjectPtr ClassStmntAST::eval(EnvPtr env) {
  ClassInfoPtr newClass = std::make_shared<ClassInfo>(shared_from_this(), env);
  env->put(name(), newClass);
  return newClass;
}

std::string ClassStmntAST::info() {
  std::string parent = superClassName();
  return "(class " + name() + ": " + parent + "| body: " + body()->info() + ")";
}

/***********************类的域访问(.xx)***********************/

Dot::Dot(): PostfixAST(ASTKind::LIST_DOT, false) {}

std::string Dot::name() {
  if (children_.empty())
    throw ASTException("not found dot target name");

  //如果.new则认为准备创建实例
  if (children_[0]->kind_ == ASTKind::LIST_NEW)
    return "new";

  //其它则认为访问某个域中的变量
  auto nameLeaf = std::dynamic_pointer_cast<ASTLeaf>(children_[0]);
  if (nameLeaf == nullptr)
    throw ASTException("get dot target name failed, unknown type for first child");
  return nameLeaf->getToken()->getText();
}

std::string Dot::info() {
  return "." + name();
}

ObjectPtr Dot::eval(__attribute__((unused))EnvPtr env, ObjectPtr caller) {
  std::string member = name();
  if (caller->kind_ == ObjKind::CLASS_INFO) {
    if (member == "new") {
      NewASTPtr newAST = std::dynamic_pointer_cast<NewAST>(children_[0]);
      return newAST->eval(env, caller);
    }
    else {
      throw ASTEvalException("unknow operation for class: " + member);
    }
  } 
  else if (caller->kind_ == ObjKind::CLASS_INSTANCE) {
    auto instance = std::dynamic_pointer_cast<ClassInstance>(caller);
    return instance->read(member);
  }
  else if (caller->kind_ == ObjKind::ENV) {
    EnvPtr env = std::dynamic_pointer_cast<CommonEnv>(caller);
    return env->get(member);
  }
  else {
    throw ASTEvalException("bad member access: " + member);
  }
}

/**********************类new创建实例***************************/

NewAST::NewAST(): PostfixAST(ASTKind::LIST_NEW, false) {}

ObjectPtr NewAST::eval(__attribute__((unused))EnvPtr env, ObjectPtr caller) {
  InstancePtr newObject = newInstance(std::dynamic_pointer_cast<ClassInfo>(caller));
  auto initFunc = newObject->read("init");   //获取初始化函数
  auto arguments = getArguments();
  //调用初始化函数，并返回新的对象
  arguments->eval(newObject->getEnvironment(), initFunc);
  return newObject;
}

ArgumentsPtr NewAST::getArguments() const {
  if (children_.empty())
    throw ASTException("get new arguments failed, not found child AST");
  if (children_[0]->kind_ != ASTKind::LIST_ARGUMENTS)
    throw ASTException("get new arguments failed, invalid child AST type");
  return std::dynamic_pointer_cast<ArgumentsAST>(children_[0]);
}

//利用闭包的方式来实现对象，每个对象有一个自己的环境
InstancePtr NewAST::newInstance(ClassInfoPtr ci) {
  EnvPtr instanceEnv = std::make_shared<MapEnv>(ci->getEnvitonment());
  InstancePtr obj = std::make_shared<ClassInstance>(instanceEnv);
  initInstance(ci, instanceEnv);
  return obj;
}

void NewAST::initInstance(ClassInfoPtr ci, EnvPtr env) {
  EnvPtr superEnv = nullptr;
  //递归由父类往下初始化
  if (ci->superClass() != nullptr) {
    superEnv = std::make_shared<MapEnv>(ci->superClass()->getEnvitonment());
    env->put("super", superEnv);
    initInstance(ci->superClass(), superEnv);
  }

  ci->body()->eval(env);    //把类元信息中的语句在对象的环境中执行
  if (superEnv != nullptr) {
    //要执行完了eval才能更改外部环境，否则覆盖了父类的同名方法，导致出bug
    env->setOuterEnv(superEnv);  
  }
  //如果未定义init函数，则抛出异常
  try {
    env->get("init");
  } catch (EnvException e) {
    throw EnvException("class " + ci->name() + " not found init function");
  }
}

/*******************return表达式*****************************/

ReturnAST::ReturnAST(): ASTList(ASTKind::LIST_RETURN, false) {}

ObjectPtr ReturnAST::eval(EnvPtr env) {
  if (children_.empty())
    throw ASTEvalException("invalid return statement");

  ObjectPtr result = children_[0]->eval(env);
  throw ASTReturnException(result);
}

std::string ReturnAST::info() {
  if (children_.empty())
    throw ASTEvalException("invalid return statement");
  
  return "return " + children_[0]->info();
}

/*******************数组字面量*******************************/

ArrayLiteralAST::ArrayLiteralAST(): ASTList(ASTKind::LIST_ARRAY_LITERAL, false) {}

int ArrayLiteralAST::size() {
  return numChildren();
}

ObjectPtr ArrayLiteralAST::eval(EnvPtr env) {
  size_t arraySize = size();
  ArrayPtr array = std::make_shared<Array>(arraySize);
  for (size_t i = 0; i < arraySize; ++i) {
    array->set(i, children_[i]->eval(env));
  }
  return array;
}

/******************数组访问后缀****************************/

ArrayRefAST::ArrayRefAST(): PostfixAST(ASTKind::LIST_ARRAY_REF, false) {}

ASTreePtr ArrayRefAST::index() {
  return children_[0];
}

std::string ArrayRefAST::info() {
  return "[" + index()->info() + "]";
}

ObjectPtr ArrayRefAST::eval(EnvPtr env, ObjectPtr caller) {
  if (caller->kind_ == ObjKind::Array) {
    ArrayPtr array = std::dynamic_pointer_cast<Array>(caller);
    ObjectPtr i = index()->eval(env);
    if (i->kind_ != ObjKind::INT) {
      throw ASTEvalException("index not a valid int type");
    }
    else {
      IntObjectPtr index = std::dynamic_pointer_cast<IntObject>(i);
      return array->get(index->value_);
    }
  }
  else {
    throw ASTEvalException("bad type for array index calling");
  }
}
