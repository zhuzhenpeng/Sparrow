#include "code.h"

unsigned Code::add() {
  return push(ADD);
}

unsigned Code::sub() {
  return push(SUB);
}

unsigned Code::mul() {
  return push(MUL);
}

unsigned Code::div() {
  return push(DIV);
}

unsigned Code::mod() {
  return push(MOD);
}

unsigned Code::eq() {
  return push(EQ);
}

unsigned Code::lt() {
  return push(LT);
}

unsigned Code::bt() {
  return push(BT);
}

unsigned Code::le() {
  return push(LE);
}

unsigned Code::be() {
  return push(BE);
}

unsigned Code::neq() {
  return push(NEQ);
}

unsigned Code::sconst(unsigned index) {
  push(SCONST);
  return push(index);
}

unsigned Code::iconst(unsigned index) {
  push(ICONST);
  return push(index);
}

unsigned Code::fconst(unsigned index) {
  push(FCONST);
  return push(index);
}

unsigned Code::call(unsigned paramNum) {
  push(CALL);
  return push(paramNum);
}

unsigned Code::ret() {
  return push(RET);
}

unsigned Code::br(unsigned index) {
  push(BR);
  return push(index);
}

unsigned Code::brt(unsigned index) {
  push(BRT);
  return push(index);
}

unsigned Code::brf(unsigned index) {
  push(BRF);
  return push(index);
}

unsigned Code::andLogic() {
  return push(AND);
}

unsigned Code::orLogic() {
  return push(OR);
}

unsigned Code::gload(unsigned index) {
  push(GLOAD);
  return push(index);
}

unsigned Code::gstore(unsigned index) {
  push(GSTORE);
  return push(index);
}

unsigned Code::cload(unsigned index) {
  push(CLOAD);
  return push(index);
}

unsigned Code::cstore(unsigned index) {
  push(CSTORE);
  return push(index);
}

unsigned Code::load(unsigned index) {
  push(LOAD); 
  return push(index);
}

unsigned Code::store(unsigned index) {
  push(STORE);
  return push(index);
}

unsigned Code::arrayGenerate(unsigned size) {
  push(ARRAY_GENERATE);
  return push(size);
}

unsigned Code::arrayAccess() {
  return push(ARRAY_ACCCESS);
}

unsigned Code::lamb(unsigned lambSrcIndex) {
  push(LAMB);
  return push(lambSrcIndex);
}

unsigned Code::dotAccess() {
  return push(DOT_ACCESS);
}

unsigned Code::dotAssign() {
  return push(DOT_ASSIGN);
}

unsigned Code::rawString(unsigned index) {
  push(RAW_STRING);
  return push(index);
}

unsigned Code::newInstance() {
  return push(NEW_INSTANCE);
}

unsigned Code::neg() {
  return push(NEG);
}

unsigned Code::halt() {
  return push(HALT);
}

std::vector<unsigned>& Code::getCodes() {
  return codes_;
}

unsigned Code::push(unsigned code) {
  codes_.push_back(code);
  return codes_.size() - 1;
}

unsigned Code::nextPosition() {
  return codes_.size();
}

void Code::set(size_t index, unsigned code) {
  codes_[index] = code;
}

unsigned Code::get(size_t index) const {
  return codes_[index];
}

unsigned Code::getCodeSize() {
  return codes_.size();
}
