#include "vm.h"

#include <cmath>
#include "code.h"
#include "../symbols.h"
#include "../ast_list.h"
#include "../pre_process/lamb_src.h"
#include "../debugger.h"

/*****************************栈帧************************************/

StackFrame::StackFrame(FuncPtr funcObj):outerNames_(funcObj->getOuterNames()) {
  env_ = funcObj->runtimeEnv();
  codes_ = funcObj->getCodes();
  if (codes_ == nullptr)
    MyDebugger::print("null codes", __FILE__, __LINE__);
  else
    MyDebugger::print(codes_->getCodeSize(), __FILE__, __LINE__);
  codeSize_ = codes_->getCodeSize();
  ip_ = 0;
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
  if (outerNames_->empty()) {
    MyDebugger::print("empty outer names", __FILE__, __LINE__);
  }
  return env_->get((*outerNames_)[nameIndex]);
}

ObjectPtr StackFrame::getLocalObj(unsigned index) {
  return env_->get(index);
}

void StackFrame::setOuterObj(unsigned nameIndex, ObjectPtr obj) {
  env_->put((*outerNames_)[nameIndex], obj);
}

void StackFrame::setLocalObj(unsigned index, ObjectPtr obj) {
  env_->put(index, obj);
}

EnvPtr StackFrame::getEnv() const {
  return env_;
}

std::string StackFrame::getNames(unsigned index) {
  if (index >= outerNames_->size())
    throw VMException("index out of range while getting raw string");
  return (*outerNames_)[index];
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

ByteCodeInterpreter::ByteCodeInterpreter(FuncPtr entry) {
  callStack_ = std::make_shared<CallStack>();
  operandStack_ = std::make_shared<OperandStack>();
  callStack_->push(std::make_shared<StackFrame>(entry));
}

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
      //MyDebugger::print("finish", __FILE__, __LINE__);
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
        //有可能是普通函数，也有可能是原生函数
        ObjectPtr funcObj = operandStack_->getAndPop();

        if (funcObj->kind_ == ObjKind::FUNCTION) {
          FuncPtr func = std::dynamic_pointer_cast<FuncObject>(funcObj);

          //如果调用函数是没有编译过的，需要运行时编译
          if (!func->isCompile())
            func->compile();

          //新建一个栈帧，并初始化它的形参
          StackFramePtr newStackFrame = std::make_shared<StackFrame>(func);
          newStackFrame->initParams(params);
          //压入调用栈，下一轮循环将执行新的函数
          callStack_->push(newStackFrame);
          break;
        }
        else if (funcObj->kind_ == ObjKind::NATIVE_FUNC) {
          //MyDebugger::print("Native Func Call", __FILE__, __LINE__);
          NativeFuncPtr func = std::dynamic_pointer_cast<NativeFunction>(funcObj);
          ObjectPtr result = func->invoke(params);
          if (result != nullptr)
            operandStack_->push(result);
          break;
        }
        else {
          MyDebugger::print(static_cast<int>(funcObj->kind_), __FILE__, __LINE__);
          throw VMException("Invalid type for function call");
        }
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
      case AND:
      {
        ObjectPtr a = operandStack_->getAndPop();
        ObjectPtr b = operandStack_->getAndPop();
        if (a->kind_ != ObjKind::BOOL || b->kind_ != ObjKind::BOOL)
          throw VMException("Invalid Logic Type for AND");
        BoolObjectPtr aCond = std::dynamic_pointer_cast<BoolObject>(a);
        BoolObjectPtr bCond = std::dynamic_pointer_cast<BoolObject>(b);
        if (aCond->b_ && bCond->b_)
          operandStack_->push(std::make_shared<BoolObject>(true));
        else
          operandStack_->push(std::make_shared<BoolObject>(false));
        break;
      }
      case OR:
      {
        ObjectPtr a = operandStack_->getAndPop();
        ObjectPtr b = operandStack_->getAndPop();
        if (a->kind_ != ObjKind::BOOL || b->kind_ != ObjKind::BOOL)
          throw VMException("Invalid Logic Type for OR");
        BoolObjectPtr aCond = std::dynamic_pointer_cast<BoolObject>(a);
        BoolObjectPtr bCond = std::dynamic_pointer_cast<BoolObject>(b);
        if (aCond->b_ || bCond->b_)
          operandStack_->push(std::make_shared<BoolObject>(true));
        else
          operandStack_->push(std::make_shared<BoolObject>(false));
        break;
      }
      case GLOAD:
      {
        unsigned nameIndex = stackFrame_->getCode();
        ObjectPtr target = stackFrame_->getOuterObj(nameIndex);
        operandStack_->push(target);
        break;
      }
      case GSTORE:
      {
        unsigned nameIndex = stackFrame_->getCode();
        ObjectPtr target = operandStack_->getAndPop();     
        stackFrame_->setOuterObj(nameIndex, target);
        break;
      }
      case CLOAD:
      {
        //环境并没有直接提供获取lamb变量的接口
        //此处的处理方法是获取上一层环境（一定是Lamb所在的外部函数局部环境）来获取变量        
        unsigned outerIndex = stackFrame_->getCode();
        EnvPtr outerEnv = stackFrame_->getEnv()->getOuterEnv();
        operandStack_->push(outerEnv->get(outerIndex));
        break;        
      }
      case CSTORE:
      {
        ObjectPtr target = operandStack_->getAndPop();
        unsigned outerIndex = stackFrame_->getCode();
        EnvPtr outerEnv = stackFrame_->getEnv()->getOuterEnv();
        outerEnv->put(outerIndex, target);
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
      case ARRAY_GENERATE:
      {
        unsigned arraySize = stackFrame_->getCode();
        ArrayPtr array = std::make_shared<Array>(arraySize);
        //编译时是顺序编译的，数组后面的元素先出栈
        for (int i = arraySize - 1; i >= 0; --i)
          array->set(i, operandStack_->getAndPop());
        operandStack_->push(array);
        break;
      }
      case ARRAY_ACCCESS:
      {
        ObjectPtr indexObj = operandStack_->getAndPop();
        if (indexObj->kind_ != ObjKind::INT)
          throw VMException("Invalid index type for array access");
        IntObjectPtr index = std::dynamic_pointer_cast<IntObject>(indexObj);
        ObjectPtr arrayObj = operandStack_->getAndPop();
        if (arrayObj->kind_ != ObjKind::Array)
          throw VMException("Invalid array type for array access");
        ArrayPtr array = std::dynamic_pointer_cast<Array>(arrayObj);
        operandStack_->push(array->get(index->value_));
        break;
      }
      case ARRAY_ASSIGN:
      {
        ObjectPtr indexObj = operandStack_->getAndPop();
        if (indexObj->kind_ != ObjKind::INT)
          throw VMException("Invalid index type for array assign");
        IntObjectPtr index = std::dynamic_pointer_cast<IntObject>(indexObj);
        ObjectPtr arrayObj = operandStack_->getAndPop();
        if (arrayObj->kind_ != ObjKind::Array)
          throw VMException("Invalid array type for array assign");
        ArrayPtr array = std::dynamic_pointer_cast<Array>(arrayObj);
        array->set(index->value_, operandStack_->getAndPop());
        break;
      }
      case LAMB:
      {
        unsigned lambSrcIndex = stackFrame_->getCode();
        LambASTPtr lambAST = g_LambSrcTable->getAST(lambSrcIndex);
        FuncPtr lambFunc = lambAST->runtimeCompile(stackFrame_->getEnv());
        //需要把lamb函数的编译状态设为义编译
        lambFunc->setCompiled();
        operandStack_->push(lambFunc);
        break;
      }
      case DOT_ACCESS:
      {
        ObjectPtr targetObj = operandStack_->getAndPop();
        if (targetObj->kind_ != ObjKind::STRING)
          throw VMException("Invalid dot access");
        StrObjectPtr target = std::dynamic_pointer_cast<StrObject>(targetObj);

        ObjectPtr callerObj = operandStack_->getAndPop();
        if (callerObj->kind_ == ObjKind::CLASS_INSTANCE) {
          auto instance = std::dynamic_pointer_cast<ClassInstance>(callerObj);
          operandStack_->push(instance->read(target->str_));
          break;
        }
        else if (callerObj->kind_ == ObjKind::ENV) {
          auto env = std::dynamic_pointer_cast<CommonEnv>(callerObj);
          operandStack_->push(env->get(target->str_));
          break;
        }
        else {
          throw VMException("UNKNOWN caller type while doing DOT ACCESS: " + target->str_);
        }
      }
      case DOT_ASSIGN:
      {
        ObjectPtr targetObj = operandStack_->getAndPop();
        if (targetObj->kind_ != ObjKind::STRING)
          throw VMException("Invalid dot access");
        StrObjectPtr target = std::dynamic_pointer_cast<StrObject>(targetObj);

        ObjectPtr callerObj = operandStack_->getAndPop();
        if (callerObj->kind_ == ObjKind::CLASS_INSTANCE) {
          auto instance = std::dynamic_pointer_cast<ClassInstance>(callerObj);
          instance->write(target->str_, operandStack_->getAndPop());
          break;
        }
        else if (callerObj->kind_ == ObjKind::ENV) {
          auto env = std::dynamic_pointer_cast<CommonEnv>(callerObj);
          env->put(target->str_, operandStack_->getAndPop());
          break;
        }
        else {
          throw VMException("UNKNOWN caller type while doing DOT ASSIGN: " + target->str_);
        }
      }
      case RAW_STRING:
      {
        unsigned index = stackFrame_->getCode();
        StrObjectPtr str = std::make_shared<StrObject>(stackFrame_->getNames(index));
        operandStack_->push(str);
        break;
      }
      case NEW_INSTANCE:
      {
        //类元对象
        ObjectPtr classInfoObj = operandStack_->getAndPop();
        if (classInfoObj->kind_ != ObjKind::CLASS_INFO)
          throw VMException("Invalid caller for creaing class instance");
        ClassInfoPtr classInfo = std::dynamic_pointer_cast<ClassInfo>(classInfoObj);

        //对象的本质是一个环境，把编译过的类环境复制给它，并调用初始化函数
        EnvPtr instanceEnv = std::dynamic_pointer_cast<CommonEnv>(
                              classInfo->getComliedEnv()->copy());
        
        //创建对象，并把对象压入操作数栈
        //下一轮的栈帧是初始化对象的，结束了以后，这个新创建的对象，才会被当前栈帧所使用
        InstancePtr newInstance = std::make_shared<ClassInstance>(instanceEnv);
        operandStack_->push(newInstance);
        
        //如果未定义init函数，则抛出异常
        try {
          instanceEnv->get("init");
        } catch (EnvException &e) {
          throw VMException("class " + classInfo->name() + " not found init function");
        }
        ObjectPtr initFuncObj = newInstance->read("init");
        if (initFuncObj->kind_ != ObjKind::FUNCTION)
          throw VMException("Invalid init member in class: " + classInfo->name());
        FuncPtr initFunc = std::dynamic_pointer_cast<FuncObject>(initFuncObj);

        //把初始函数压入栈，下一轮循环会执行CALL指令
        operandStack_->push(initFunc);
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
