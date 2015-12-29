#include "lexer_imp.h"

LexerImp::LexerImp() {
  std::string patternStr = \
  "\\s*((//.*)|([0-9]+)|(\"(\\\\\"|\\\\\\\\|\\\\n|[^\"])*\")"
  "|[A-Z_a-z][A-Z_a-z0-9]*|==|<=|>=|&&|\\|\\||\\p{Punct})?";

  parsePattern_ = std::regex(patternStr);
}

void LexerImp::parseText(const std::string &text) {
  
}

void LexerImp::parseFile(const std::string &fileName) {
  
}

std::string LexerImp::getFileName() const {
  return fileName_;
}

//read总是返回队列最前端的Token，并把它出列
//如果队列为空，如果hasMore_仍为真，责尝试解析源的下一行；否则返回EOF Token
TokenPtr LexerImp::read() {
  
}

//peek总是返回队列中第k+1个Token
//如果超出队列中的数量，则解析至足够的数量；如果仍然不够，则返回EOF Token
TokenPtr LexerImp::peek(int k) {
  
}

////////////////////////////////////// private methods

void LexerImp::parseLine(const std::string &str) {
   
}

