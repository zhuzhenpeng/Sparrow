#include "vm.h"


/*****************************栈帧************************************/

StackFrame::StackFrame(ArrayEnvPtr runtimeEnv, CodePtr codes): 
  env_(runtimeEnv), ip_(0), codes_(codes) {
  codeSize_ = codes_->getCodeSize();  
}

unsigned StackFrame::getCode() {
  if (ip_ >= codeSize_)
    throw VMException("fatal error: memory out of range");
  return codes_->get(ip_++);
}

void StackFrame::setIp(unsigned ip) {
  ip_ = ip;
}


/****************************调用栈*********************************/

CallStack::CallStack() = default;

bool CallStack::empty() const {
  return stack_.empty();
}

void CallStack::push(StackFramePtr stackFrame) {
  stack_.push(stackFrame);
}

StackFramePtr CallStack::top() const {
  return stack_.top();
}

void CallStack::pop() {
  stack_.pop();
}

/************************操作数栈*********************************/

OperandStack::OperandStack() = default;

bool OperandStack::empty() const {
  return stack_.empty(); 
}

void OperandStack::clear() {
  while (!stack_.empty())
    stack_.pop();
}

void OperandStack::push(ObjectPtr obj) {
  stack_.push(obj);
}

ObjectPtr OperandStack::top() const {
  return stack_.top();
}

void OperandStack::pop() {
  stack_.pop();
}

/***********************字节码解释器****************************/

ByteCodeInterpreter::ByteCodeInterpreter(CallStackPtr callStack, OperandStackPtr operandStack):
  callStack_(callStack), operandStack_(operandStack) {}

void ByteCodeInterpreter::run() {
  while (!callStack_->empty()) {
    
  }
}
