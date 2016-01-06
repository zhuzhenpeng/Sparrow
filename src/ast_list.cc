#include "ast_list.h"

ASTList::ASTList(ASTKind kind): ASTree(kind) {}

ASTreePtr ASTList::child(int i) {
  return children_[i];
}

int ASTList::numChildren() {
  return children_.size();
}

Iterator<ASTreePtr> ASTList::children() {
  return ASTListIterator<ASTreePtr>(children_);
}

std::string ASTList::info() {
  std::string result; 
  for(auto child: children_) {
    result += child->info();
    result += "\n";
  }
  return result;
}
