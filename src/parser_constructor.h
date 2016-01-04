#ifndef SPARROW_PARSER_CONSTRUCTOR_H_
#define SPARROW_PARSER_CONSTRUCTOR_H_

/* 利用组合子的编程思想编写一个语法解析器构造器 */
#include <vector>
#include <memory>
#include "lexer.h"
#include "ast_tree.h"

class Parser;
using ParserPtr = std::shared_ptr<Parser>;

//所有的parser的操作规则都继承于这个接口,子类命名都以PR结尾
class ParseRule {
public:
  virtual void parse(Lexer &lexer, std::vector<ASTreePtr> &ast) = 0;
  virtual bool match(Lexer &lexer) = 0;
};
using ParseRulePtr = std::shared_ptr<ParseRule>;

//parser的普通操作
class CommonParsePR: public ParseRule {
public:
  CommonParsePR(ParserPtr parser);
  void parse(Lexer &lexer, std::vector<ASTreePtr> &ast) override;
  bool match(Lexer &lexer) override;
private:
  ParserPtr parser_;
};

//Parser类
//外部通过调用Parser的一些接口可以构造出特性规则的Parser
class Parser {
public:
  ASTreePtr parse(Lexer &lexer);
  bool match(Lexer &lexer);
private:
  std::vector<ParseRulePtr> rulesCombination_;
};

#endif
