#include "symbols.h"

Symbols::Symbols(SymbolsPtr outer, bool isFunc): 
   isFuncSymbols_(isFunc), outer_(outer) {}

size_t Symbols::getRuntimeIndex(const std::string &name) {
  //a.变量名是$开头的，返回-1
  //b.如果该符号表不是函数的，那么把变量插入，返回-1
  //c.如果该符号表是函数的，且在当前环境找到，返回相应下标
  //d.如果该符号表是函数的，在当前环境找不到，尝试向上寻找，如果找到：
  //  上层环境是全局返回-1，上层环境是函数环境返回-2;
  //  否则在当前符号表插入并返回相应下标  
  
  if (name[0] == '$')
    return -1;

  if (!isFuncSymbols_) {
    symbolsIndex_.insert({name, -1});
    return -1;
  }
  
  if (symbolsIndex_.find(name) != symbolsIndex_.end()) {
    return symbolsIndex_[name];
  } else {
    SymbolsPtr varLocation = locateSymbol(outer_, name);
    if (varLocation == nullptr) {
      size_t index = symbolsIndex_.size();
      symbolsIndex_.insert({name, index});
      return index;
    }
    else {
      if (varLocation->isFuncSymbols_)
        return -2;
      else
        return -1;
    }
  }
}

SymbolsPtr Symbols::locateSymbol(SymbolsPtr symbol, const std::string &name) {
  if (symbol == nullptr)
    return nullptr;

  if (symbol->symbolsIndex_.find(name) != symbol->symbolsIndex_.end())
    return symbol;
  else
    return locateSymbol(symbol->outer_, name);
}
