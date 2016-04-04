#include "lamb_src.h"

unsigned LambSrcTable::put(std::shared_ptr<LambAST> lambAST) {
  table_.push_back(lambAST);
  return table_.size() - 1;
}

std::shared_ptr<LambAST> LambSrcTable::getAST(unsigned index) {
  if (index >= table_.size())
    throw NotFoundLambSrcException("Lamb src index out of range");
  return table_[index];
}

LambSrcTablePtr g_LambSrcTable = std::make_shared<LambSrcTable>();
