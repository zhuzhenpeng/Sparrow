#ifndef SPARROW_NATIVE_FUNC_H_
#define SPARROW_NATIVE_FUNC_H_

#include "env.h"
#include "symbols.h"

//初始化
class NativeFuncInitializer {
public:
  static void initialize(EnvPtr unit, SymbolsPtr symbols);
};

//打印函数
class NativePrint: public NativeFunction {
public:
  NativePrint();
  ObjectPtr invoke(const std::vector<ObjectPtr> &params) override;
};

#endif
