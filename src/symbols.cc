#include "symbols.h"

/**********************三种类型的全局符号表************************/
IntSymbolsPtr g_IntSymbols = std::make_shared<ConstantSymbols<int>>();

FloatSymbolsPtr g_FloatSymbols = std::make_shared<ConstantSymbols<double>>();

StrSymbolsPtr g_StrSymbols = std::make_shared<ConstantSymbols<std::string>>();

/**************************通用符号表******************************/
Symbols::Symbols(SymbolsPtr outer, SymbolsKind kind):
   kind_(kind), outer_(outer) {}

int Symbols::getRuntimeIndex(const std::string &name) {
  //a.变量名是$开头的，返回-1
  //b.如果该符号表不是函数的，那么把变量插入，返回-1
  //c.如果该符号表是函数的，且在当前环境找到，返回相应下标
  //d.如果该符号表是函数的，在当前环境找不到，尝试向上寻找，如果找到：
  //  上层环境是全局返回-1，上层环境是函数环境返回（-2 - 下标）
  //  否则在当前符号表插入并返回相应下标  
  
  if (name[0] == '$')
    return -1;

  if (kind_ != SymbolsKind::FUNCTION) {
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
      if (varLocation->kind_ == SymbolsKind::FUNCTION)
        return (-2 - varLocation->getRuntimeIndex(name));
      else
        return -1;
    }
  }
}

size_t Symbols::getSymbolSize() const {
  return symbolsIndex_.size();
}

void Symbols::putClassSymbols(const std::string &className, 
    SymbolsPtr symbols) {
  if (kind_ != SymbolsKind::UNIT)
    throw SymbolsException("cannot put class symbols into un-unit symbols");
  classSymbols_.insert({className, symbols});
}

SymbolsPtr Symbols::getClassSymbols(const std::string &className) {
  if (kind_ != SymbolsKind::UNIT)
    throw SymbolsException("cannot get class symbols from un-unit symbols");
  if (classSymbols_.find(className) != classSymbols_.end())
    return classSymbols_[className];
  else
    throw SymbolsException("not found class: [" + className + "] symbols");
}

SymbolsPtr Symbols::locateSymbol(SymbolsPtr symbol, const std::string &name) {
  if (symbol == nullptr)
    return nullptr;

  if (symbol->symbolsIndex_.find(name) != symbol->symbolsIndex_.end())
    return symbol;
  else
    return locateSymbol(symbol->outer_, name);
}
