#ifndef SPARROW_AST_LIST_H_
#define SPARROW_AST_LIST_H_

#include "ast_tree.h"
#include <vector>

/**************************AST内部（非叶子）节点******************************/

class ASTList: public ASTree {

  /*****************内部迭代器类********************/
  template <typename Item>
  class ASTListIterator: public Iterator<Item> {
  public:
    ASTListIterator(const std::vector<Item> &items): items_(items) {};

    void first() override {
      index_ = 0;
    }

    void next() override {
      ++index_;
    }

    bool hasNext() const override {
      return (index_ + 1) < items_.size();
    }

    Item current() const override {
      return items_[index_];
    }

  private:
    const std::vector<Item> &items_;
    size_t index_ = 0;
  };

public:
  ASTList(ASTKind kind, bool ignore);

  //返回第i个子节点
  ASTreePtr child(int i) override;

  //返回子节点数量
  int numChildren() override;

  //返回子节点迭代器
  Iterator<ASTreePtr> iterator() override;

  //返回该节点信息
  std::string info() override;

  //抛出异常
  ObjectPtr eval(EnvPtr env) override;

  void preProcess(SymbolsPtr symbols) override;

  std::vector<ASTreePtr>& children();

  bool ignore() const;

protected:
  std::vector<ASTreePtr> children_;
  
  //考虑在没有子节点和子节点个数为1的情况下，该节点是否可以被忽略
  //用于剪枝
  bool ignore_;
};

/****************************use导入类*************************************/
class UsingAST: public ASTList {
public:
  UsingAST();

  void preProcess(SymbolsPtr symbols) override;

  ObjectPtr eval(EnvPtr env) override;

  std::string info() override;

private:
  //导入的类的别名
  std::string alias();

  //导入模块源
  std::string srcUnit();

  //导入的类名
  std::string srcClassName();
};

/**************************元表达式****************************************/
class PostfixAST;
class PrimaryExprAST: public ASTList {
public:
  PrimaryExprAST();
  ObjectPtr eval(EnvPtr env) override;

  //nest表示从外往内数的第几层，如果是最外(右)层，则为0
  //用于.作用域访问或数组的下表访问
  std::shared_ptr<PostfixAST> postfix(size_t nest);
  bool hasPostfix(size_t nest);

  //返回操作数（该节点的第一个对象）
  ASTreePtr operand();
  
  //计算子表达式，如a.b.c则从左往右依次计算
  ObjectPtr evalSubExpr(EnvPtr env, size_t nest);

  void compile() override;

  void compileSubExpr(size_t nest);
};
using PrimaryExprPtr = std::shared_ptr<PrimaryExprAST>;

/**************************负值表达式*************************************/

class NegativeExprAST: public ASTList {
public:
  NegativeExprAST();
  std::string info() override;
  ObjectPtr eval(EnvPtr env) override;
  void compile() override;
};

/*****************************二元表达式***********************************/

class BinaryExprAST: public ASTList {
public:
  BinaryExprAST();
  ASTreePtr leftFactor();
  ASTreePtr rightFactor();
  std::string getOperator();
  ObjectPtr eval(EnvPtr env) override;
  void compile() override;
private:
  //赋值操作，仅当操作符为等号时
  ObjectPtr assignOp(EnvPtr env, ObjectPtr rightValue);

  //除赋值以外其它运算符操作
  ObjectPtr otherOp(ObjectPtr left, const std::string &op, ObjectPtr right);

  //int型间的运算
  ObjectPtr computeInt(int left, const std::string &op, int right);

  //float型间的运算
  ObjectPtr computeFloat(double left, const std::string &op, double right);

  //对象字段赋值
  ObjectPtr setInstanceField(InstancePtr obj, const std::string &filedName, ObjectPtr rvalue);

  //除了赋值操作符以外的运算符编译成字节码
  void compileOtherOp(const std::string &op);
  
private:
  void checkValid();
};

/********************************块**************************************/

class BlockStmntAST: public ASTList {
public:
  BlockStmntAST();
  ObjectPtr eval(EnvPtr env) override;
  void compile() override;
};
using BlockStmntPtr = std::shared_ptr<BlockStmntAST>;

/*****************************条件判断**********************************/

class ConditionStmntAST: public ASTList {
public:
  ConditionStmntAST();
  //该节点作为虚节点存在，总是只有一个子节点，打印信息时把子节点信息打印即可
  std::string info() override;
  ObjectPtr eval(EnvPtr env) override;
  void compile() override;
};
using ConditionStmntPtr =  std::shared_ptr<ConditionStmntAST>;

/******************************与逻辑***********************************/

class AndLogicAST: public ASTList {
public:
  AndLogicAST();
  ConditionStmntPtr leftExpr();
  ConditionStmntPtr rightExpr();
  std::string info() override;
  ObjectPtr eval(EnvPtr env) override;
  void compile() override;
};
using AndLogicPtr = std::shared_ptr<AndLogicAST>;

/*****************************或逻辑************************************/

class OrLogicAST: public ASTList {
public:
  OrLogicAST();
  ConditionStmntPtr leftExpr();
  ConditionStmntPtr rightExpr();
  std::string info() override;
  ObjectPtr eval(EnvPtr env) override;
  void compile() override;
};
using OrLogicPtr = std::shared_ptr<OrLogicAST>;

/******************************if块*************************************/

class IfStmntAST: public ASTList {
public:
  IfStmntAST();
  ConditionStmntPtr condition();
  ASTreePtr thenBlock();
  ASTreePtr elseBlock();
  std::string info() override;
  ObjectPtr eval(EnvPtr env) override;
  void compile() override;

  //if的条件跳转在字节码中的位置
  size_t ifBrfPosition;
  
  //thenblock结束后无条件跳转在字节码中的位置
  size_t blockBrPosition;

  //elseblock的起始地址
  size_t elseblockPosition;

  //if块字节码的结束地址（elseblock的下一个位置）
  size_t endPosition;

private:
  //判断是否有elif块，如果没有返回0，如果有返回数量
  unsigned countElifBlock();
};

/****************************elif块************************************/

class ElifStmntAST: public ASTList {
public:
  ElifStmntAST();
  ConditionStmntPtr condition();
  ASTreePtr thenBlock();
  std::string info() override;
  ObjectPtr eval(EnvPtr env) override;
  void compile() override;
  
  //condition的起始地址
  size_t conditionPosition;

  //elif跳转位置在字节码中的地址
  size_t elifBrfPosition;

  //thenblock结束后无条件跳转在字节码中的位置
  size_t blockBrPosition;
};
using ElifStmntPtr = std::shared_ptr<ElifStmntAST>;

/****************************while块***********************************/

class WhileStmntAST: public ASTList {
public:
  WhileStmntAST();
  ConditionStmntPtr condition();
  ASTreePtr body();
  std::string info() override;
  ObjectPtr eval(EnvPtr env) override;
  void compile() override;
};

/****************************Null块************************************/

class NullStmntAST: public ASTList {
public:
  NullStmntAST();
  ObjectPtr eval(EnvPtr env) override;
};

/****************************形参列表*********************************/

class ParameterListAST: public ASTList {
public:
  ParameterListAST();
  std::string paramName(size_t i);
  size_t size() const;

  //为形参分配空间
  void preProcess(SymbolsPtr symbols) override;

  //继承自父类的eval接口废弃，抛出异常
  ObjectPtr eval(__attribute__((unused)) EnvPtr env) override {
    throw ASTEvalException("error call for Parameter List AST eval, abandoned");
  }

  //根据入参设置函数运行时环境
  //该语法树记录形参名字，实参数据由外部args传入
  void eval(EnvPtr funcEnv, EnvPtr callerEnv, const std::vector<ASTreePtr> &args);

private:
  //形参位置
  std::vector<size_t> paramsOffset_;
};
using ParameterListPtr = std::shared_ptr<ParameterListAST>;

/**************************函数定义**********************************/

class DefStmntAST: public ASTList {
public:
  DefStmntAST();
  std::string funcName();
  ParameterListPtr parameterList();
  BlockStmntPtr block();
  std::string info() override;

  void compile() override;

  //确定运行时环境局部变量的大小
  void preProcess(SymbolsPtr symbols) override;

  //生成一个函数对象，放入当前的环境中
  //返回空指针
  ObjectPtr eval(EnvPtr env) override;

  //获取函数运行时环境的局部变量所占空间大小
  static size_t getLocalVarSize(SymbolsPtr outer, ParameterListPtr params, 
      BlockStmntPtr block);

private:
  //函数局部变量所需大小
  size_t localVarSize_;
};

/************************后缀表达式接口*****************************/

class PostfixAST: public ASTList {
public:
  PostfixAST(ASTKind kind, bool ignore): ASTList(kind, ignore) {}

  //Postfix继承自父类的eval方法废弃，会抛出异常，改用其它eval方法
  ObjectPtr eval(__attribute__((unused)) EnvPtr env) override {
    throw ASTEvalException("Postfix AST eval exception, abandoned");
  }
  
  //caller指的是使用这个后缀的对象
  virtual ObjectPtr eval(EnvPtr env, ObjectPtr caller) = 0;
};

/***************************函数实参********************************/

class ArgumentsAST: public PostfixAST {
public:
  ArgumentsAST();
  size_t size() const;
  
  //函数调用发生在这里
  //caller为函数对象
  ObjectPtr eval(EnvPtr env, ObjectPtr caller) override;

  //逆序把实参压入栈，并压入调用指令
  void compile() override;

private:
  //调用原生函数
  ObjectPtr invokeNative(EnvPtr env, NativeFuncPtr func);
};
using ArgumentsPtr = std::shared_ptr<ArgumentsAST>;

/**************************闭包**********************************/

class LambAST: public ASTList, public std::enable_shared_from_this<LambAST> {
public:
  LambAST();
  ParameterListPtr parameterList();
  BlockStmntPtr block();
  std::string info() override;

  //确定函数运行时环境局部变量的大小
  void preProcess(SymbolsPtr symbols) override;

  //返回一个函数对象
  ObjectPtr eval(EnvPtr env) override;

  void compile() override;

  //运行时编译，并返回一个函数对象
  FuncPtr runtimeCompile(EnvPtr env);

private:
  //函数运行时环境的局部变量大小
  size_t localVarSize_;

  //闭包在源码表中的位置
  unsigned srcIndex_;
};
using LambASTPtr = std::shared_ptr<LambAST>;

/*************************类************************************/

class ClassBodyAST: public ASTList {
public:
  ClassBodyAST();
  ObjectPtr eval(EnvPtr env) override;
};
using ClassBodyPtr = std::shared_ptr<ClassBodyAST>;

class ClassStmntAST: public ASTList, public std::enable_shared_from_this<ClassStmntAST> {
public:
  ClassStmntAST();

  //返回类的名字
  std::string name();

  //返回父类的名字
  std::string superClassName();

  //返回类定义体
  ClassBodyPtr body();

  //将类名放到全局符号表中
  void preProcess(SymbolsPtr symbols) override;

  ObjectPtr eval(EnvPtr env) override;

  std::string info() override;
};
using ClassStmntPtr = std::shared_ptr<ClassStmntAST>;

/**************************域访问(.xx)***********************/

class Dot: public PostfixAST {
public:
  Dot();

  //访问目标的名字
  std::string name();

  std::string info() override;

  //如果caller是类元信息的实例，创建某个类的实例，
  //如果是对象的实例，访问某个对象的成员变量、方法
  //否则报错
  ObjectPtr eval(EnvPtr env, ObjectPtr caller) override;

  void compile() override;

  void compileAssign();
};
using DotPtr = std::shared_ptr<Dot>;

/**********************类new创建实例***************************/

class NewAST: public PostfixAST{
public:
  NewAST(); 

  //caller是类元信息
  //函数返回一个新创建的实例
  ObjectPtr eval(EnvPtr env, ObjectPtr caller) override;

  void compile() override;

private:
  ArgumentsPtr getArguments() const;

  //创建并初始化对象
  InstancePtr newInstance(ClassInfoPtr ci);

  //初始化对象，env是对象自身的环境
  void initInstance(ClassInfoPtr ci, EnvPtr env);
};
using NewASTPtr = std::shared_ptr<NewAST>;

/*******************return表达式*****************************/

class ReturnAST: public ASTList {
public:
  ReturnAST();

  //计算出结果后抛出异常给上层处理
  ObjectPtr eval(EnvPtr env) override;

  std::string info() override;

  void compile() override;
};
using ReturnASTPtr = std::shared_ptr<ReturnAST>;

class ASTReturnException: public ASTEvalException {
public:
  ASTReturnException(ObjectPtr result): ASTEvalException("return"), result_(result) {}
  ObjectPtr result_;
};

/*******************数组字面量*******************************/

class ArrayLiteralAST: public ASTList {
public:
  ArrayLiteralAST();
  int size();
  
  //生成定长数组
  ObjectPtr eval(EnvPtr env) override;

  void compile() override;
};

/******************数组访问后缀****************************/

class ArrayRefAST: public PostfixAST {
public:
  ArrayRefAST();
  ASTreePtr index();
  std::string info() override;

  //访问指定下标的对象
  ObjectPtr eval(EnvPtr env, ObjectPtr caller) override;

  void compile() override;
};
using ArrayRefPtr = std::shared_ptr<ArrayRefAST>;

#endif
