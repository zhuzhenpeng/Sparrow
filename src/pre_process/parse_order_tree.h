#ifndef SPARROW_PARSE_ORDER_TREE_H_
#define SPARROW_PARSE_ORDER_TREE_H_

#include <vector>
#include <string>
#include <memory>
#include <map>

/*解析顺序树
 * 预处理根据require指令，根据导入unit作为当前unit的子节点的规则构造解析顺序的树
 * 对于已经存在于树中的unit（以绝对路径作为标识），不再插入
 *
 * 外部只需后序遍历即可正确地执行源程序语义
 */

/*********************************节点***********************************/
struct ParseOrderTreeNode;
using ParseOrderTreeNodePtr = std::shared_ptr<ParseOrderTreeNode>;
struct ParseOrderTreeNode {
  std::string absolutePath;   //绝对路径，也是唯一标识
  std::vector<ParseOrderTreeNodePtr> children;  //子节点
};


/********************************树**************************************/
class ParseOrderTree {
public:
  ParseOrderTree(const std::string &headAbsolutePath);

  //对指定父节点插入子节点
  //如果子节点的绝对路径已经在这棵树中出现，则不插入
  void putNew(const std::string &parentAbPath, const std::string &childAbPath);

  //返回树的根节点
  ParseOrderTreeNodePtr getRoot();

  //判断某个节点是否已经在树中
  bool hasNode(const std::string &path);

private:
  ParseOrderTreeNodePtr head_;    //根，程序运行时的指定入口
  std::map<std::string, ParseOrderTreeNodePtr> units_; //单元集，以绝对路径作为标识
};
using ParseOrderTreePtr = std::shared_ptr<ParseOrderTree>;


#endif
