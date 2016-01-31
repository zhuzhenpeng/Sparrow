#include "ast_list.h"
#include "ast_leaf.h"
#include "debugger.h"

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
    // 类似于 foo(2)这种调用，ASTLeaf的eval，从环境中搜索foo的Obj并把它返回
    // 在此处它就是caller
    ObjectPtr caller = evalSubExpr(env, nest + 1);
    result = postfix(nest)->eval(env, caller);
  }
  else {
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
    env->put(std::dynamic_pointer_cast<IdTokenAST>(leftTree)->getId(), rightValue);
    return rightValue;
  }
  else if (leftTree->kind_ == ASTKind::LIST_PRIMARY_EXPR) {
    PrimaryExprPtr primary = std::dynamic_pointer_cast<PrimaryExprAST>(leftTree);

    //对象域访问
    if (primary->hasPostfix(0) && primary->postfix(0)->kind_ == ASTKind::LIST_INSTANCE_DOT) {
      ObjectPtr obj = primary->evalSubExpr(env, 1);
      if (obj->kind_ == ObjKind::CLASS_INSTANCE) {
        auto dot = std::dynamic_pointer_cast<InstanceDot>(primary->postfix(0));
        return setInstanceField(std::dynamic_pointer_cast<ClassInstance>(obj), 
            dot->name(), rightValue);
      }
      else {
        throw ASTEvalException("bad assign, left value is not a valid class instance");
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

ObjectPtr IfStmntAST::eval(EnvPtr env) {
  auto b = condition()->eval(env);
  if (b->kind_ != ObjKind::BOOL)
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

WhileStmntAST::WhileStmntAST(): ASTList(ASTKind::LIST_WHILE_STMNT, false) {}

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

void ParameterListAST::eval(EnvPtr funcEnv, EnvPtr callerEnv, 
    const std::vector<ASTreePtr> &args) {
  for (size_t i = 0; i < args.size(); ++i) {
    funcEnv->put(paramName(i), args[i]->eval(callerEnv));
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

ObjectPtr DefStmntAST::eval(EnvPtr env) {
  auto funcObj = std::make_shared<FuncObject>(funcName(), parameterList(), block(), env);
  env->put(funcName(), funcObj);
  return nullptr;
}

/***************************实参***********************************/

Arguments::Arguments(): PostfixAST(ASTKind::LIST_ARGUMENTS, false) {}

size_t Arguments::size() const {
  return children_.size();
}

ObjectPtr Arguments::eval(EnvPtr env, ObjectPtr caller) {
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

  //每次调用函数，都有创建一个新的内部环境
  EnvPtr funcEnv = func->runtimeEnv();
  params->eval(funcEnv, env, children_);
  auto result = func->block()->eval(funcEnv);
  return result;
}

ObjectPtr Arguments::invokeNative(EnvPtr env, NativeFuncPtr func) {
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

ObjectPtr LambAST::eval(EnvPtr env) {
  //lambda创建的闭包都用closure来表示它的函数名
  return std::make_shared<FuncObject>("closure", parameterList(), block(), env);
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

InstanceDot::InstanceDot(): PostfixAST(ASTKind::LIST_INSTANCE_DOT, false) {}

std::string InstanceDot::name() {
  auto nameLeaf = std::dynamic_pointer_cast<ASTLeaf>(children_[0]);
  if (nameLeaf == nullptr)
    throw ASTException("get dot target name failed, unknown type for first child");
  return nameLeaf->getToken()->getText();
}

std::string InstanceDot::info() {
  return "." + name();
}

ObjectPtr InstanceDot::eval(__attribute__((unused))EnvPtr env, ObjectPtr caller) {
  std::string member = name();
  if (caller->kind_ == ObjKind::CLASS_INFO) {
    if (member == "new")
      return newInstance(std::dynamic_pointer_cast<ClassInfo>(caller));
    else
      throw ASTEvalException("unknow operation for class: " + member);
  } 
  else if (caller->kind_ == ObjKind::CLASS_INSTANCE) {
    auto instance = std::dynamic_pointer_cast<ClassInstance>(caller);
    return instance->read(member);
  }
  else {
    throw ASTEvalException("bad member access: " + member);
  }
}

//利用闭包的方式来实现对象
ObjectPtr InstanceDot::newInstance(ClassInfoPtr ci) {
  EnvPtr instanceEnv = std::make_shared<Environment>(ci->getEnvitonment());
  InstancePtr obj = std::make_shared<ClassInstance>(instanceEnv);
  initInstance(ci, instanceEnv);
  return obj;
}

void InstanceDot::initInstance(ClassInfoPtr ci, EnvPtr env) {
  //递归由父类往下初始化
  if (ci->superClass() != nullptr)
    initInstance(ci->superClass(), env);
  ci->body()->eval(env);
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
