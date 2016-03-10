#include "ast_leaf.h"

#include "symbols.h"

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

//void ASTLeaf::preProcess(__attribute__((unused))SymbolsPtr symbols) {
  //throw ASTPreProcessException("error call: unimplement for AST preprocess");
//}

TokenPtr ASTLeaf::getToken() const {
  return token_;
}

void ASTLeaf::setIndex(size_t index) {
  index_ = index;
}

size_t ASTLeaf::getIndex() const {
  return index_;
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

void IntTokenAST::preProcess(__attribute__((unused))SymbolsPtr symbols) {
  index_ = g_IntSymbols->getIndex(getValue());
}

/*********************FloatToken对应的叶子节点************************/

FloatTokenAST::FloatTokenAST(TokenPtr token): ASTLeaf(ASTKind::LEAF_FLOAT, token) {}

std::string FloatTokenAST::info() {
  return std::to_string(getValue());
}

ObjectPtr FloatTokenAST::eval(__attribute__((unused))EnvPtr env) {
  return std::make_shared<FloatObject>(getValue());
}

double FloatTokenAST::getValue() const {
  return std::static_pointer_cast<FloatToken>(token_)->getValue();
}

void FloatTokenAST::preProcess(__attribute__((unused))SymbolsPtr symbols) {
  index_ = g_FloatSymbols->getIndex(getValue());
}

/*********************IdToken对应的叶子节点***************************/

IdTokenAST::IdTokenAST(TokenPtr token): ASTLeaf(ASTKind::LEAF_Id, token) {}

std::string IdTokenAST::info() {
  return getId();
}

ObjectPtr IdTokenAST::eval(EnvPtr env) {
  if (kind_ == IdKind::LOCAL) {
    //MyDebugger::print("local", __FILE__, __LINE__);
    //MyDebugger::print(getId());
    return env->get(index_);
  }
  else if (kind_ == IdKind::CLOSURE) {
    EnvPtr outerFuncEnv = env->getOuterEnv();
    if (outerFuncEnv == nullptr)
      throw ASTEvalException("null outer function env");
    //MyDebugger::print("closure", __FILE__, __LINE__);
    return outerFuncEnv->get(index_);
  }
  else {
    //MyDebugger::print("global", __FILE__, __LINE__);
    return env->get(getId());
  } 
}

void IdTokenAST::assign(EnvPtr env, ObjectPtr value) {
  if (kind_ == IdKind::LOCAL) {
    env->put(index_, value);
  }
  else if (kind_ == IdKind::CLOSURE) {
    EnvPtr outerFuncEnv = env->getOuterEnv();
    if (outerFuncEnv == nullptr)
      throw ASTEvalException("null outer function env");
    outerFuncEnv->put(index_, value);
  }
  else {
    env->put(getId(), value);
  }
}

std::string IdTokenAST::getId() const {
  return token_->getText();
}

void IdTokenAST::preProcess(SymbolsPtr symbols) {
  int result = symbols->getRuntimeIndex(getId());
  if (result == -1) {
    kind_ = IdKind::GLOBAL;
  }
  else if (result >= 0) {
    kind_ = IdKind::LOCAL;
    index_ = result;
  }
  else {
    kind_ = IdKind::CLOSURE;
    index_ = result + 2;    //该变量在上层环境的位置
  }
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

void StrTokenAST::preProcess(__attribute__((unused))SymbolsPtr symbols) {
  index_ = g_StrSymbols->getIndex(getContent());
}
