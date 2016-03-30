#include "vm.h"

#include <cmath>
#include "code.h"
#include "../symbols.h"

/*****************************栈帧************************************/

StackFrame::StackFrame(FuncPtr funcObj):outerNames_(funcObj->getOuterNames()) {
  env_ = funcObj->runtimeEnv();
  codes_ = funcObj->getCodes();
  codeSize_ = codes_->getCodeSize();
}

void StackFrame::initParams(const std::vector<ObjectPtr> &arguments) {
  //局部环境中的头几个对象是参数
  //和入参的顺序相反
  unsigned localIndex = 0;
  for (auto iter = arguments.rbegin(); iter != arguments.rend(); ++iter) {
    env_->put(localIndex, *iter);
    ++localIndex;
  }
}

unsigned StackFrame::getCode() {
  if (ip_ >= codeSize_)
    throw VMException("fatal error: memory out of range");
  return codes_->get(ip_++);
}

void StackFrame::setIp(unsigned ip) {
  ip_ = ip;
}

bool StackFrame::isEnd() {
  return ip_ >= codeSize_;
}

ObjectPtr StackFrame::getOuterObj(unsigned nameIndex) {
  return env_->get(outerNames_[nameIndex]);
}

ObjectPtr StackFrame::getLocalObj(unsigned index) {
  return env_->get(index);
}

void StackFrame::setOuterObj(unsigned nameIndex, ObjectPtr obj) {
  env_->put(outerNames_[nameIndex], obj);
}

void StackFrame::setLocalObj(unsigned index, ObjectPtr obj) {
  env_->put(index, obj);
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
  if (stack_.empty())
    throw VMException("Call Stack is empty while calling top");
  else
    return stack_.top();
}

void CallStack::pop() {
  stack_.pop();
}

StackFramePtr CallStack::getAndPop() {
  if (stack_.empty()) {
    throw VMException("Call Stack is empty while calling get and pop");
  }
  else {
    StackFramePtr result = stack_.top();
    stack_.pop();
    return result;
  }
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
  if (stack_.empty())
    throw VMException("Operand Stack is empty while calling top");
  else
    return stack_.top();
}

void OperandStack::pop() {
  stack_.pop();
}

ObjectPtr OperandStack::getAndPop() {
  if (stack_.empty()) {
    throw VMException("Operand Stack is empty while calling get and pop");
  }
  else {
    ObjectPtr result = stack_.top();
    stack_.pop();
    return result;
  }
}

/***********************字节码解释器****************************/

ByteCodeInterpreter::ByteCodeInterpreter(CallStackPtr callStack, OperandStackPtr operandStack):
  callStack_(callStack), operandStack_(operandStack) {}

void ByteCodeInterpreter::arithmeticTypeCast(ObjectPtr a, ObjectPtr b, Instruction op) {
  if (a->kind_ == ObjKind::INT && b->kind_ == ObjKind::INT) {
    IntObjectPtr aInt = std::dynamic_pointer_cast<IntObject>(a);
    IntObjectPtr bInt = std::dynamic_pointer_cast<IntObject>(b);
    intArithmeticOp(aInt, bInt, op);
  } 
  else if (a->kind_ == ObjKind::FLOAT && b->kind_ == ObjKind::FLOAT) {
    FloatObjectPtr aFloat = std::dynamic_pointer_cast<FloatObject>(a);
    FloatObjectPtr bFloat = std::dynamic_pointer_cast<FloatObject>(b);
    floatArithmeticOp(aFloat, bFloat, op);
  }
  else if (a->kind_ == ObjKind::INT && b->kind_ == ObjKind::FLOAT) {
    double aValue = std::dynamic_pointer_cast<IntObject>(a)->value_;
    FloatObjectPtr aFloat = std::make_shared<FloatObject>(aValue);
    FloatObjectPtr bFloat = std::dynamic_pointer_cast<FloatObject>(b);
    floatArithmeticOp(aFloat, bFloat, op);
  }
  else if (a->kind_ == ObjKind::FLOAT && b->kind_ == ObjKind::INT) {
    FloatObjectPtr aFloat = std::dynamic_pointer_cast<FloatObject>(a);
    double bValue = std::static_pointer_cast<IntObject>(b)->value_;
    FloatObjectPtr bFloat = std::make_shared<FloatObject>(bValue);
    floatArithmeticOp(aFloat, bFloat, op);
  }
  else if (a->kind_ == ObjKind::STRING && b->kind_ == ObjKind::STRING){
    StrObjectPtr aStr = std::dynamic_pointer_cast<StrObject>(a);
    StrObjectPtr bStr = std::dynamic_pointer_cast<StrObject>(b);
    strArithmeticOp(aStr, bStr, op);
  }
  else {
    //类型不合法，抛出异常
    throw VMException("Invalid type of 2 params for arithmetic operation");
  }
}

void ByteCodeInterpreter::intArithmeticOp(IntObjectPtr a, IntObjectPtr b, Instruction op) {
  switch (op) {
    case ADD:
      operandStack_->push(std::make_shared<IntObject>(a->value_ + b->value_));
      break;
    case SUB:
      operandStack_->push(std::make_shared<IntObject>(a->value_ - b->value_));
      break;
    case MUL:
      operandStack_->push(std::make_shared<IntObject>(a->value_ * b->value_));
      break;
    case DIV:
      operandStack_->push(std::make_shared<IntObject>(a->value_ / b->value_));
      break;
    case EQ:
      operandStack_->push(std::make_shared<BoolObject>(a->value_ == b->value_));
      break;
    case LT:
      operandStack_->push(std::make_shared<BoolObject>(a->value_ < b->value_));
      break;
    case BT:
      operandStack_->push(std::make_shared<BoolObject>(a->value_ > b->value_));
      break;
    case LE:
      operandStack_->push(std::make_shared<BoolObject>(a->value_ <= b->value_));
      break;
    case BE:
      operandStack_->push(std::make_shared<BoolObject>(a->value_ >= b->value_));
      break;
    case NEQ:
      operandStack_->push(std::make_shared<BoolObject>(a->value_ != b->value_));
      break;
    default:
      //操作符不合法，抛出异常
      throw VMException("Invalid operation for integer");
  }
}

void ByteCodeInterpreter::floatArithmeticOp(FloatObjectPtr a, FloatObjectPtr b, Instruction op) {
  switch (op) {
    case ADD:
      operandStack_->push(std::make_shared<FloatObject>(a->value_ + b->value_));
      break;
    case SUB:
      operandStack_->push(std::make_shared<FloatObject>(a->value_ - b->value_));
      break;
    case MUL:
      operandStack_->push(std::make_shared<FloatObject>(a->value_ * b->value_));
      break;
    case DIV:
      operandStack_->push(std::make_shared<FloatObject>(a->value_ / b->value_));
      break;
    case EQ:
      {
        double diff = std::abs(a->value_ - b->value_);
        operandStack_->push(std::make_shared<BoolObject>(diff <= 1e-10));
        break;
      }
    case LT:
      operandStack_->push(std::make_shared<BoolObject>(a->value_ < b->value_));
      break;
    case BT:
      operandStack_->push(std::make_shared<BoolObject>(a->value_ > b->value_));
      break;
    case LE:
      operandStack_->push(std::make_shared<BoolObject>(a->value_ <= b->value_));
      break;
    case BE:
      operandStack_->push(std::make_shared<BoolObject>(a->value_ >= b->value_));
      break;
    case NEQ:
      {
        double diff = std::abs(a->value_ - b->value_);
        operandStack_->push(std::make_shared<BoolObject>(diff > 1e-10));
        break;
      }
    default:
      //非法操作符，抛出异常
      throw VMException("Invalid operation for float");
  }
}

void ByteCodeInterpreter::strArithmeticOp(StrObjectPtr a, StrObjectPtr b, Instruction op) {
  switch (op) {
    case ADD:
      operandStack_->push(std::make_shared<StrObject>(a->str_ + b->str_));
      break;
    case EQ:
      operandStack_->push(std::make_shared<BoolObject>(a->str_ == b->str_));
      break;
    case NEQ:
      operandStack_->push(std::make_shared<BoolObject>(a->str_ != b->str_));
      break;
    default:
      throw VMException("Invalid operation for string");
  }
}

void ByteCodeInterpreter::run() {
  while (!callStack_->empty()) {
    stackFrame_ = callStack_->top();

    //对于不带return语句的函数，运行到最后一句时函数结束
    if (stackFrame_->isEnd()) {
      callStack_->pop();
      continue;
    }
    unsigned code = stackFrame_->getCode();
    switch (code) {
      case ADD:
      case SUB:
      case MUL:
      case DIV:
      case MOD:
      case EQ:
      case LT:
      case BT:
      case LE:
      case BE:
      case NEQ:
      {
        ObjectPtr a = operandStack_->getAndPop();
        ObjectPtr b = operandStack_->getAndPop();
        arithmeticTypeCast(a, b, static_cast<Instruction>(code));
        break;
      }
      case SCONST:
      {
        unsigned index = stackFrame_->getCode();
        operandStack_->push(std::make_shared<StrObject>(g_StrSymbols->get(index)));
        break;
      }
      case ICONST:
      {
        unsigned index = stackFrame_->getCode();
        operandStack_->push(std::make_shared<IntObject>(g_IntSymbols->get(index)));
        break;
      }
      case FCONST:
      {
        unsigned index = stackFrame_->getCode();
        operandStack_->push(std::make_shared<FloatObject>(g_FloatSymbols->get(index)));
        break;
      }
      case CALL:
      {
        //实参个数
        unsigned paramsNum = stackFrame_->getCode();
        //获得实参
        std::vector<ObjectPtr> params;
        for (size_t i = 0; i < paramsNum; ++i)
          params.push_back(operandStack_->getAndPop());
        //获得函数对象
        ObjectPtr funcObj = operandStack_->getAndPop();
        if (funcObj->kind_ != ObjKind::FUNCTION)
          throw VMException("Invalid type for function call");
        FuncPtr func = std::dynamic_pointer_cast<FuncObject>(funcObj);
        //新建一个栈帧，并初始化它的形参
        StackFramePtr newStackFrame = std::make_shared<StackFrame>(func);
        newStackFrame->initParams(params);
        //压入调用栈，清空操作数栈，下一轮循环将执行新的函数
        callStack_->push(newStackFrame);
        operandStack_->clear();
        break;
      }
      case RET:
      {
        callStack_->pop();
        break;
      }
      case BR:
      {
        unsigned position = stackFrame_->getCode();
        stackFrame_->setIp(position);
        break;
      }
      case BRT:
      {
        unsigned position = stackFrame_->getCode();
        ObjectPtr condObj = operandStack_->getAndPop();
        if (condObj->kind_ != ObjKind::BOOL)
          throw VMException("Invald type for predicate");
        BoolObjectPtr cond = std::dynamic_pointer_cast<BoolObject>(condObj);
        if (cond->b_)
          stackFrame_->setIp(position);
        break;
      }
      case BRF:
      {
        unsigned position = stackFrame_->getCode();
        ObjectPtr condObj = operandStack_->getAndPop();
        if (condObj->kind_ != ObjKind::BOOL)
          throw VMException("Invald type for predicate");
        BoolObjectPtr cond = std::dynamic_pointer_cast<BoolObject>(condObj);
        if (!cond->b_)
          stackFrame_->setIp(position);
        break;
      }
      case GLOAD:
      case CLOAD:
      {
        unsigned nameIndex = stackFrame_->getCode();
        ObjectPtr target = stackFrame_->getOuterObj(nameIndex);
        operandStack_->push(target);
        break;
      }
      case GSTORE:
      case CSTORE:
      {
        unsigned nameIndex = stackFrame_->getCode();
        ObjectPtr target = operandStack_->getAndPop();     
        stackFrame_->setOuterObj(nameIndex, target);
        break;
      }
      case LOAD:
      {
        unsigned index = stackFrame_->getCode();
        ObjectPtr target = stackFrame_->getLocalObj(index);
        operandStack_->push(target);
        break;
      }
      case STORE:
      {
        unsigned index = stackFrame_->getCode();
        ObjectPtr target = operandStack_->getAndPop();
        stackFrame_->setLocalObj(index, target);
        break;
      }
      case NATIVE_CALL:
      {
        //unsigned nativeFuncIndex = stackFrame_->getCode();
        throw VMException("Unimplement native call");
      }
      case NIL:
      {
        operandStack_->push(nullptr);
        break;
      }
      case POP:
      {
        operandStack_->pop();
        break;
      }
      case NEG:
      {
        ObjectPtr obj = operandStack_->getAndPop();
        if (obj->kind_ == ObjKind::INT) {
          IntObjectPtr intObj = std::dynamic_pointer_cast<IntObject>(obj);
          IntObjectPtr negObj = std::make_shared<IntObject>(-intObj->value_);
          operandStack_->push(negObj);
          break;
        }
        else if (obj->kind_ == ObjKind::FLOAT) {
          FloatObjectPtr floatObj = std::dynamic_pointer_cast<FloatObject>(obj);
          FloatObjectPtr negObj = std::make_shared<FloatObject>(-floatObj->value_);
          operandStack_->push(negObj);
          break;
        }
        else {
          throw VMException("Invalid type for NEG");
        }
      }
      case HALT:
      {
        return;
      }
      default:
      {
        throw VMException("UNKNOWN CODE");
      }
    }
  }
}
