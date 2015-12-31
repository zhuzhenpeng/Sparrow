#ifndef SPARROW_AST_TREE_H_
#define SPARROW_AST_TREE_H_

#include <memory>
#include <string>

/*
 * AST迭代器接口
 */
template <typename Item>
class Itertor {
public:
  virtual void first() = 0;
  virtual void next() = 0;
  virtual bool hasNext() const = 0;
  virtual Item current() const = 0;
};

/*
 * AST的基类，用于定义接口
 */
class ASTree;
using ASTreePtr = std::shared_ptr<ASTree>;

class ASTree {
public:
  //获取下标为i的子节点，
  virtual ASTreePtr child(int i) = 0;

  //返回子节点数量
  virtual int numChildren() = 0;

  //返回子节点集合的迭代器
  virtual Itertor<ASTreePtr> children() = 0;

  //获取该节点的信息
  virtual std::string info() = 0;
};


#endif
