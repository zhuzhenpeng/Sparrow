#ifndef SPARROW_LEXER_IMP_H_
#define SPARROW_LEXER_IMP_H_

#include <string>
#include <deque>
#include <iostream>
#include <regex>
#include "lexer.h"

class LexerImp: public Lexer {
public:
  LexerImp();

  ~LexerImp();

  void parseText(const std::string &text) override;

  void parseFile(const std::string &fileName) override;

  std::string getFileName() const override;

  TokenPtr read() override;

  TokenPtr peek(int k) override;

private:
  //填充队列直至可以访问到第i+1项
  bool fillQueue(size_t i);
  
  //解析下一行，结果放进队列中
  void parseNextLine();

  //重置内部状态
  void resetStatus();

private:
  //存储解析出来的Token的队列
  std::deque<TokenPtr> tokenQueue_;

  //判断是否已经解析完
  bool hasMore_ = true;

  //当前解析到的行数
  int lineNumber_ = -1;

  //如果是解析文件，需要记录下文件名
  //可以根据当前文件名是否为空来判断lexer正在解些文件还是文本
  std::string fileName_;

  //如果是解析文字，需要把源文本记录下来
  std::string srcText_;

  //解析流
  std::istream *is_;

  //暂时用正则表达式来进行解些
  std::regex parsePattern_;
};

#endif
