#ifndef SPARROW_VM_H_
#define SPARROW_VM_H_

/**虚拟机运行时所需的组件
 * 1.栈帧，每次发生函数调用时，都将一个栈帧压入调用栈，栈帧存储着该函数运行时局部变量，
 *   指令计数器，以及对函数字节码的引用
 * 2.调用栈，存储栈帧的地方
 * 3.操作数栈，任何一个指令的操作数都存放在此
 * 4.字节码解释器
 */

#include <string>
#include <exception>
#include <stack>
#include <memory>
#include "../env.h"

/*****************************异常************************************/
class VMException: std::exception {
public:
  VMException(const std::string &msg): errMsg_(msg) {}
  const char* what() const noexcept {
    return errMsg_.c_str();
  }
private:
  std::string errMsg_;
};

/*****************************栈帧************************************/

class StackFrame {
public:
  StackFrame(FuncPtr funcObj);

  //初始化形参
  void initParams(const std::vector<ObjectPtr> &arguments);

  //获取当前的指令，计数器加一
  unsigned getCode();

  //设置计数器
  void setIp(unsigned ip);

  //是否已经结束
  bool isEnd();

  //获取非局部变量
  ObjectPtr getOuterObj(unsigned nameIndex);

  //获取局部变量
  ObjectPtr getLocalObj(unsigned index);

  //设置非局部变量
  void setOuterObj(unsigned nameIndex, ObjectPtr obj);

  //设置局部变量
  void setLocalObj(unsigned index, ObjectPtr obj); 

  //返回运行时的局部环境
  EnvPtr getEnv() const;

  //获取名字字符
  std::string getNames(unsigned index);

private:
  //运行时局部环境
  EnvPtr env_;

  //指令计数器
  unsigned ip_;

  //字节码
  CodePtr codes_;

  //字节码的长度
  unsigned codeSize_;

  //非局部变量的名称
  std::shared_ptr<std::vector<std::string>> outerNames_;
};
using StackFramePtr = std::shared_ptr<StackFrame>;

/****************************调用栈*********************************/

class CallStack {
public:
  CallStack(); 

  //是否为空
  bool empty() const;

  //压入
  void push(StackFramePtr stackFrame);

  //返回栈顶元素
  StackFramePtr top() const;

  //弹出栈顶元素
  void pop();

  //返回栈顶元素，并弹出
  StackFramePtr getAndPop();

private:
  std::stack<StackFramePtr> stack_;
};
using CallStackPtr = std::shared_ptr<CallStack>;

/************************操作数栈*********************************/

class OperandStack {
public:
  OperandStack();

  //是否为空
  bool empty() const;

  //清空栈
  void clear();

  //压入栈
  void push(ObjectPtr obj);

  //返回栈顶元素
  ObjectPtr top() const;

  //弹出栈顶元素
  void pop();

  //返回栈顶元素，并弹出
  ObjectPtr getAndPop();

private:
  std::stack<ObjectPtr> stack_;
};
using OperandStackPtr = std::shared_ptr<OperandStack>;

/***********************字节码解释器****************************/

class ByteCodeInterpreter {
public:
  ByteCodeInterpreter(FuncPtr entry);

  void run();

private:
  //运算时类型转换
  void arithmeticTypeCast(ObjectPtr a, ObjectPtr b, Instruction op);

  //整型运算操作
  void intArithmeticOp(IntObjectPtr a, IntObjectPtr b, Instruction op);

  //浮点型运算操作
  void floatArithmeticOp(FloatObjectPtr a, FloatObjectPtr b, Instruction op);

  //字符串型运算操作
  void strArithmeticOp(StrObjectPtr a, StrObjectPtr b, Instruction op);

private: 
  CallStackPtr callStack_;

  OperandStackPtr operandStack_;

  StackFramePtr stackFrame_;
};

#endif
