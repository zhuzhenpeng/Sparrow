#ifndef SPARROW_SYMBOLS_H_
#define SPARROW_SYMBOLS_H_

#include <string>
#include <map>
#include <vector>
#include <memory>

/*符号表：
 * 在遍历AST时，保存常量在常量池的位置信息
 * 记录局部变量在函数运行时环境的位置信息
 */

/**************************常量符号表******************************/
template <typename T>
class ConstantSymbols {
public:
  ConstantSymbols() = default;

  //获取常量在常量池的地址，如果不存在则放入其中
  size_t getIndex(T constant) {
    if (symbolsIndex_.find(constant) != symbolsIndex_.end()) {
      return symbolsIndex_[constant];
    }
    else {
      constPool_.push_back(constant);
      size_t result = constPool_.size() - 1;
      symbolsIndex_[constant] = result;
      return result;
    }
  }

  //获取常量池
  std::vector<T> &getConstantPool() const {
    return constPool_;
  }

private:
  //常量下标
  std::map<T, size_t> symbolsIndex_;

  //常量池
  std::vector<T> constPool_;
};
using IntSymbolsPtr = std::shared_ptr<ConstantSymbols<int>>;
using FloatSymbolsPtr = std::shared_ptr<ConstantSymbols<double>>;
using StrSymbolsPtr = std::shared_ptr<ConstantSymbols<std::string>>;

/**************************通用符号表******************************/
class Symbols;
using SymbolsPtr = std::shared_ptr<Symbols>;

class Symbols {
public:
  Symbols(SymbolsPtr outer, bool isFunc);

  //获取局部变量在函数运行时环境的位置
  //如果该变量为全局变量，则返回-1
  //如果该变量为闭包上层函数的临时变量，返回-2
  size_t getRuntimeIndex(const std::string &name);

private:
  //定位符号所在的符号表，如果找不到返回空指针
  SymbolsPtr locateSymbol(SymbolsPtr symbol, const std::string &name);
  
private:
  //是否是函数的符号表
  bool isFuncSymbols_;

  //上一层符号表
  SymbolsPtr outer_;

  //变量以及它保存的位置，即使是全局变量也会分配位置（这些位置信息是没用的）
  std::map<std::string, size_t> symbolsIndex_;
};
#endif
