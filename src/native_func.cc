#include "native_func.h"

#include <iostream>
#include "env.h"

std::vector<NativeFuncPtr> NativeFuncInitializer::nativeFuncs_;

void NativeFuncInitializer::initialize(EnvPtr unit, SymbolsPtr symbols) {
  for (auto func: nativeFuncs_) {
    unit->put(func->name(), func);
    symbols->getRuntimeIndex(func->name());
  }
}

void NativeFuncInitializer::addToInitList(NativeFuncPtr func) {
  nativeFuncs_.push_back(func);
}

/***************************普通打印函数*******************************/

NativePrint::NativePrint(): NativeFunction("print", 1) {}

ObjectPtr NativePrint::invoke(const std::vector<ObjectPtr> &params) {
  ObjectPtr param = params[0];
  if (param == nullptr)
    throw NativeFuncException("ERROR! Invalid for null variable for print");
  switch (param->kind_) {
    case ObjKind::INT:
      std::cout << std::dynamic_pointer_cast<IntObject>(param)->value_;
      break;
    case ObjKind::FLOAT:
      std::cout << std::dynamic_pointer_cast<FloatObject>(param)->value_;
      break;
    case ObjKind::STRING:
      std::cout << std::dynamic_pointer_cast<StrObject>(param)->str_;
      break;
    case ObjKind::BOOL:
      std::cout << std::dynamic_pointer_cast<BoolObject>(param)->b_;
      break;
    default:
      std::cout << param->info() << std::endl;
      break;
  }
  return nullptr;
}

/***************************换行打印函数******************************/

NativePrintLine::NativePrintLine(): NativeFunction("printLine", 1) {}

ObjectPtr NativePrintLine::invoke(const std::vector<ObjectPtr> &params) {
  ObjectPtr param = params[0];
  if (param == nullptr)
    throw NativeFuncException("ERROR! Invalid for null variable for print");
  switch (param->kind_) {
    case ObjKind::INT:
      std::cout << std::dynamic_pointer_cast<IntObject>(param)->value_ << std::endl;
      break;
    case ObjKind::FLOAT:
      std::cout << std::dynamic_pointer_cast<FloatObject>(param)->value_ << std::endl;
      break;
    case ObjKind::STRING:
      std::cout << std::dynamic_pointer_cast<StrObject>(param)->str_ << std::endl;
      break;
    case ObjKind::BOOL:
      std::cout << std::dynamic_pointer_cast<BoolObject>(param)->b_ << std::endl;
      break;
    default:
      std::cout << param->info() << std::endl;
      break;
  }
  return nullptr;
}

/************************从标准输入读取一个整型**********************/

NativeReadInt::NativeReadInt(): NativeFunction("readInt", 0) {}

ObjectPtr NativeReadInt::invoke(const std::vector<ObjectPtr> &__attribute__((unused))params) {
  int num;
  std::cin >> num;
  return std::make_shared<IntObject>(num);
}

/**********************从标准输入读取一个浮点型*********************/

NativeReadFloat::NativeReadFloat(): NativeFunction("readFloat", 0) {}

ObjectPtr NativeReadFloat::invoke(const std::vector<ObjectPtr> &__attribute__((unused))params) {
  double num;
  std::cin >> num;
  return std::make_shared<FloatObject>(num);
}

/**********************从标准输入读取一个字符串********************/

NativeReadString::NativeReadString(): NativeFunction("readStr", 0) {}

ObjectPtr NativeReadString::invoke(const std::vector<ObjectPtr> &__attribute__((unused))params) {
  std::string str;
  std::cin >> str;
  return std::make_shared<StrObject>(str);
}

