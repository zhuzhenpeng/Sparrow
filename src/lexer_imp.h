#ifndef SPARROW_LEXER_IMP_H_
#define SPARROW_LEXER_IMP_H_

#include <string>
#include <deque>
#include <fstream>
#include <regex>
#include "lexer.h"

class LexerImp: public Lexer {
public:
  LexerImp();

  void parseText(const std::string &text) override;

  void parseFile(const std::string &fileName) override;

  std::string getFileName() const override;

  TokenPtr read() override;

  TokenPtr peek(int k) override;

private:
  //逐行解析，并把结果放进队列中
  void parseLine(const std::string &str);

private:
  //存储解析出来的Token的队列
  std::deque<Token> tokenQueue_;

  //判断是否已经解析完
  bool hasMore = true;

  //当前解析到的行数
  int lineNumber = -1;

  //如果是解析文件
  std::ifstream srcFile_;
  std::string fileName_;

  //如果是解析文字
  std::string srcText_;

  //暂时用正则表达式来进行解些
  std::regex parsePattern_;
};

#endif
