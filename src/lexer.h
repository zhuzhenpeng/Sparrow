#ifndef SPARROW_LEXER_H_
#define SPARROW_LEXER_H_

#include <string>
#include "token.h"

//lexer接口
class Lexer {
public:
  //解析文字
  virtual void parseText(const std::string &text) = 0;

  //解析文件
  virtual void parseFile(const std::string &fileName) = 0;

  //获取输入源文件名字
  virtual std::string getFileName() const = 0;

  //返回输入流中的下一个Token
  virtual TokenPtr read() = 0;

  //返回输入流中第k+1个Token
  virtual TokenPtr peek(int k) = 0;
};

#endif
