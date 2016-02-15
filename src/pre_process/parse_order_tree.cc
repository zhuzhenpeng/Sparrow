#include "parse_order_tree.h"

#include "preprocess_exception.h"

ParseOrderTree::ParseOrderTree(const std::string &headAbsolutePath) {
  head_ = std::make_shared<ParseOrderTreeNode>();
  head_->absolutePath = headAbsolutePath;
  units_[headAbsolutePath] = head_;
}

void ParseOrderTree::putNew(const std::string &parentAbPath, const std::string &childAbPath) {
  //父节点必须存在，而子节点不存在
  if (units_.find(parentAbPath) == units_.end())
    throw ParseOrderTreeException("error happen while put new child: null parent");
  if (units_.find(childAbPath) != units_.end())
    throw ParseOrderTreeException("fatal error, repeat parsed unit");

  ParseOrderTreeNodePtr childNode = std::make_shared<ParseOrderTreeNode>();
  childNode->absolutePath = childAbPath;
  units_[parentAbPath]->children.push_back(childNode);

  //把新产生的节点记录下来
  units_.insert({childAbPath, childNode});
}

ParseOrderTreeNodePtr ParseOrderTree::getRoot() {
  return head_;
}

bool ParseOrderTree::hasNode(const std::string &path) {
  return units_.find(path) != units_.end();
}
