#ifndef SPARROW_PARSER_CONSTRUCTOR_H_
#define SPARROW_PARSER_CONSTRUCTOR_H_

/* 利用组合子的编程思想编写一个语法解析器构造器 */
#include <vector>
#include <memory>
#include <exception>
#include <string>
#include <set>
#include "lexer.h"
#include "ast_tree.h"

class Parser;
using ParserPtr = std::shared_ptr<Parser>;

/***************************生成各类AST的静态工厂******************************/
class ASTFactory {
public:
  static ASTreePtr getLeafInstance(ASTKind astKind, TokenPtr token);
  static ASTreePtr getListInstance(ASTKind astKind);
};

/******************************操作规则接口************************************/
//所有的parser的操作规则都继承于这个接口,直接子类命名都以PR结尾
class ParseRule {
public:
  virtual void parse(Lexer &lexer, std::vector<ASTreePtr> &ast) = 0;
  virtual bool match(Lexer &lexer) = 0;
};
using ParseRulePtr = std::shared_ptr<ParseRule>;

/***********************************普通操作***********************************/
class CommonParsePR: public ParseRule {
public:
  CommonParsePR(ParserPtr parser);
  void parse(Lexer &lexer, std::vector<ASTreePtr> &ast) override;
  bool match(Lexer &lexer) override;
private:
  ParserPtr parser_;
};

/*********************************选择操作************************************/
class OrParsePR: public ParseRule {
public:
public:
  OrParsePR(const std::vector<ParserPtr> &parsers);
  void parse(Lexer &lexer, std::vector<ASTreePtr> &ast) override;
  bool match(Lexer &lexer) override;
private:
  //选择机制：首个匹配原则
  ParserPtr choosePs(Lexer &lexer);
private:
  std::vector<ParserPtr> parsers_;
};

/*********************************重复操作************************************/
class RepeatParsePR: public ParseRule {
public:
  RepeatParsePR(ParserPtr parser, bool onceFlag);
  void parse(Lexer &lexer, std::vector<ASTreePtr> &ast) override;
  bool match(Lexer &lexer) override;
private:
  ParserPtr parser_;
  bool onceFlag_;   //为真是0..1的情况，否则是0..*的情况
};

/********************************匹配Token************************************/
//这类操作的match接口实现是真正的有匹配逻辑的

class MatchTokenPR: public ParseRule {
public:
  MatchTokenPR(ASTKind astKind);
  void parse(Lexer &lexer, std::vector<ASTreePtr> &ast) override;
  //没有实现match接口，由子类实现
protected:
  ASTKind kind_;
};

//id
class IdMatcher: public MatchTokenPR {
public:
  IdMatcher(std::set<std::string> &reserved);
  bool match(Lexer &lexer) override;
private:
  std::set<std::string> &reserved_;  //保留字集合，由外部维护
};

//int
class IntMatcher: public MatchTokenPR {
public:
  IntMatcher();
  bool match(Lexer &lexer) override;
};

//string
class StrMatcher: public MatchTokenPR {
public:
  StrMatcher();
  bool match(Lexer &lexer) override;
};

/*******************************自定义终结符**********************************/

/********************************Parser类*************************************/
//外部通过调用Parser的一些接口可以构造出特性规则的Parser
class Parser {
public:
  ASTreePtr parse(Lexer &lexer);
  bool match(Lexer &lexer);
private:
  std::vector<ParseRulePtr> rulesCombination_;
};

/********************************异常类***************************************/
class ParseException: public std::exception {
public:
  ParseException(const std::string &msg): errMsg_(msg) {};
  const char* what() const noexcept override {
    return errMsg_.c_str();
  }
private:
  std::string errMsg_;
};

#endif
