#ifndef SPARROW_AST_LIST_H_
#define SPARROW_AST_LIST_H_

#include "ast_tree.h"
#include <vector>

/**************************AST内部（非叶子）节点******************************/

enum class ASTListKind {
  
};

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
  ASTList(ASTKind kind);

  //返回第i个子节点
  ASTreePtr child(int i) override;

  //返回子节点数量
  int numChildren() override;

  //返回子节点迭代器
  Iterator<ASTreePtr> iterator() override;

  //返回该节点信息
  std::string info() override;

  //抛出异常
  ObjectPtr eval(Environment &env) override;

  std::vector<ASTreePtr>& children();

protected:
  std::vector<ASTreePtr> children_;
};

/**************************元表达式****************************************/

class PrimaryExprAST: public ASTList {
public:
  PrimaryExprAST();
  ObjectPtr eval(Environment &env) override;
};

/**************************负值表达式*************************************/

class NegativeExprAST: public ASTList {
public:
  NegativeExprAST();
  std::string info() override;
  ObjectPtr eval(Environment &env) override;
};

/*****************************二元表达式***********************************/

class BinaryExprAST: public ASTList {
public:
  BinaryExprAST();
  ASTreePtr leftFactor();
  ASTreePtr rightFactor();
  std::string getOperator();
  ObjectPtr eval(Environment &env) override;
private:
  //赋值操作
  ObjectPtr assignOp(Environment &env, ObjectPtr rightValue);

  //除赋值以外其它操作
  ObjectPtr otherOp(ObjectPtr left, const std::string &op, ObjectPtr right);

  //数字间的运算
  ObjectPtr computeNumber(IntObjectPtr left, const std::string &op, IntObjectPtr right);
private:
  void checkValid();
};

/********************************块**************************************/

class BlockStmntAST: public ASTList {
public:
  BlockStmntAST();
  ObjectPtr eval(Environment &env) override;
};

/******************************if块*************************************/

class IfStmntAST: public ASTList {
public:
  IfStmntAST();
  ASTreePtr condition();
  ASTreePtr thenBlock();
  ASTreePtr elseBlock();
  std::string info() override;
  ObjectPtr eval(Environment &env) override;
};

/****************************while块***********************************/

class WhileStmntAST: public ASTList {
public:
  WhileStmntAST();
  ASTreePtr condition();
  ASTreePtr body();
  std::string info() override;
  ObjectPtr eval(Environment &env) override;
};

/****************************Null块************************************/

class NullStmntAST: public ASTList {
public:
  NullStmntAST();
  ObjectPtr eval(Environment &env) override;
};

#endif
