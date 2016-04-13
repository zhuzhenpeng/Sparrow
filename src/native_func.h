#ifndef SPARROW_NATIVE_FUNC_H_
#define SPARROW_NATIVE_FUNC_H_

#include "env.h"
#include "symbols.h"
#include <vector>
#include <memory>

//初始化
//将函数放入环境和符号表中
class NativeFuncInitializer {
public:
  static void initialize(EnvPtr unit, SymbolsPtr symbols);

  //将原生函数放入初始化列表
  static void addToInitList(NativeFuncPtr func);

private:
  //初始化列表
  static std::vector<NativeFuncPtr> nativeFuncs_;
};

/***************************普通打印函数*******************************/
class NativePrint: public NativeFunction {
public:
  NativePrint();
  ObjectPtr invoke(const std::vector<ObjectPtr> &params) override;
};

/***************************换行打印函数******************************/
class NativePrintLine: public NativeFunction {
public:
  NativePrintLine();
  ObjectPtr invoke(const std::vector<ObjectPtr> &params) override;
};

/************************从标准输入读取一个整型**********************/
class NativeReadInt: public NativeFunction {
public:
  NativeReadInt();
  ObjectPtr invoke(const std::vector<ObjectPtr> &params) override;
};

/**********************从标准输入读取一个浮点型*********************/
class NativeReadFloat: public NativeFunction {
public:  
  NativeReadFloat();
  ObjectPtr invoke(const std::vector<ObjectPtr> &params) override;
};

/**********************从标准输入读取一个字符串********************/
class NativeReadString: public NativeFunction {
public:
  NativeReadString();
  ObjectPtr invoke(const std::vector<ObjectPtr> &params) override;
};

/************************返回一个只读文件*************************/
class NativeOpenROFile: public NativeFunction {
public:
  NativeOpenROFile();
  ObjectPtr invoke(const std::vector<ObjectPtr> &params) override;
};

/************************返回一个只写文件***********************/
class NativeOpenWOFile: public NativeFunction {
public:
  NativeOpenWOFile();
  ObjectPtr invoke(const std::vector<ObjectPtr> &params) override;
};

#endif
