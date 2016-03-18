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

unsigned Code::call(unsigned index) {
  push(CALL);
  return push(index);
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

unsigned Code::gload(unsigned index) {
  push(GLOAD);
  return push(index);
}

unsigned Code::gstore(unsigned index) {
  push(GSTORE);
  return push(index);
}

unsigned Code::push(unsigned code) {
  codes_.push_back(code);
  return codes_.size() - 1;
}


