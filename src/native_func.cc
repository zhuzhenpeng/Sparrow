#include "native_func.h"

#include <iostream>

void NativeFuncInitializer::initialize(EnvPtr global) {
  auto print = std::make_shared<NativePrint>();
  global->put(print->name(), print);
}

NativePrint::NativePrint(): NativeFunction("print", 1) {}

ObjectPtr NativePrint::invoke(const std::vector<ObjectPtr> &params) {
  ObjectPtr param = params[0];
  switch (param->kind_) {
    case ObjKind::Int:
      std::cout << std::dynamic_pointer_cast<IntObject>(param)->value_ << std::endl;
      break;
    case ObjKind::String:
      std::cout << std::dynamic_pointer_cast<StrObject>(param)->str_ << std::endl;
      break;
    case ObjKind::Bool:
      std::cout << std::dynamic_pointer_cast<BoolObject>(param)->b_ << std::endl;
      break;
    default:
      std::cout << param->info() << std::endl;
      break;
  }
  return nullptr;
}
