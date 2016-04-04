#ifndef SPARROW_LAMB_SRC_H_
#define SPARROW_LAMB_SRC_H_

/**闭包源码表
 * 存储闭包的AST，用于运行时编译
 */

#include <memory>
#include <vector>
#include <exception>

class LambAST;

class LambSrcTable {
public:
  LambSrcTable() = default;

  //分配一个存储位置给闭包AST
  //预处理时闭包AST根据该函数获取其相应的存储下标
  unsigned put(std::shared_ptr<LambAST> lambAST);

  //根据下标获取闭包的AST
  //虚拟机运行时根据该函数获得闭包源码
  std::shared_ptr<LambAST> getAST(unsigned index);

private:
  std::vector<std::shared_ptr<LambAST>> table_;
};
using LambSrcTablePtr = std::shared_ptr<LambSrcTable>;

//全局唯一的表
extern LambSrcTablePtr g_LambSrcTable;

//无法找到闭包的异常
class NotFoundLambSrcException: public std::exception {
public:
  NotFoundLambSrcException(const std::string &errMsg): errMsg_(errMsg) {}

  const char *what() const noexcept override {
    return errMsg_.c_str();
  }

private:
  std::string errMsg_;
};

#endif
