#include "parser_constructor.h"

#include "ast_leaf.h"
#include "ast_leaf.cc"

/***************************生成各类AST的静态工厂******************************/
ASTreePtr ASTFactory::getLeafInstance(ASTKind astKind, TokenPtr token) {
  ASTreePtr result = nullptr;
  switch (astKind) {
    case ASTKind::LEAF_Id:
       result = std::make_shared<IdTokenAST>(token);
       break;
    case ASTKind::LEAF_INT:
       result = std::make_shared<IntTokenAST>(token);
       break;
    default:
       break;
  }
  return result;
}

ASTreePtr ASTFactory::getListInstance(ASTKind astKind) {
  return nullptr;
}

/***************************commom*****************************************/

CommonParsePR::CommonParsePR(ParserPtr parser): parser_(parser) {}

void CommonParsePR::parse(Lexer &lexer, std::vector<ASTreePtr> &ast) {
  ast.push_back(parser_->parse(lexer));
}

bool CommonParsePR::match(Lexer &lexer) {
  return parser_->match(lexer);
}

/****************************or*******************************************/

OrParsePR::OrParsePR(const std::vector<ParserPtr> &parsers): parsers_(parsers) {}

void OrParsePR::parse(Lexer &lexer, std::vector<ASTreePtr> &ast) {
  ParserPtr parser = choosePs(lexer);
  if (parser == nullptr) 
    throw ParseException("or choose failed: " + lexer.peek(0)->info());
  else
    ast.push_back(parser->parse(lexer));
}

bool OrParsePR::match(Lexer &lexer) {
  return choosePs(lexer) != nullptr;
}

ParserPtr OrParsePR::choosePs(Lexer &lexer) {
  for (auto &pptr: parsers_) {
    if (pptr->match(lexer))
      return pptr;
  }
  return nullptr;
}

/***************************repeat**************************************/

RepeatParsePR::RepeatParsePR(ParserPtr parser, bool onceFlag): 
  parser_(parser), onceFlag_(onceFlag) {}

void RepeatParsePR::parse(Lexer &lexer, std::vector<ASTreePtr> &ast) {
  //至少是0次匹配
  while (parser_->match(lexer)) {
    ast.push_back(parser_->parse(lexer));
    if (onceFlag_)
      break;
  } 
}

bool RepeatParsePR::match(Lexer &lexer) {
  return parser_->match(lexer);
}

/********************************匹配Token************************************/

namespace {
  //根据ASTKind返回它的含义
  std::string getASTKindMeaning(ASTKind kind) {
    switch (kind) {
      case ASTKind::INVALID:
        return "invalid";
      case ASTKind::LEAF_INT:
        return "int token";
      case ASTKind::LEAF_Id:
        return "id token";
      case ASTKind::LEAF_STR:
        return "string token";
      default:
        return "UNKNOWN";
    }
  }
}

MatchTokenPR::MatchTokenPR(ASTKind astKind): kind_(astKind) {};

void MatchTokenPR::parse(Lexer &lexer, std::vector<ASTreePtr> &ast) {
  if (match(lexer)) {
    //通过parse后，lexer消耗一个token
    ASTreePtr leaf = ASTFactory::getLeafInstance(kind_, lexer.read());
    ast.push_back(leaf);
  }
  else {
    throw ParseException("parse error: " + lexer.peek(0)->info() + 
        " , except: " + getASTKindMeaning(kind_));
  }
}

//id
IdMatcher::IdMatcher(std::set<std::string> &reserved):
  MatchTokenPR(ASTKind::LEAF_Id), reserved_(reserved) {}

bool IdMatcher::match(Lexer &lexer) {
  auto token = lexer.peek(0);
  return token->getKind() == TokenKind::TK_ID && 
    reserved_.find(reinterpret_cast<IdToken*>(token.get())->getId()) == reserved_.end();
}

//int
IntMatcher::IntMatcher(): MatchTokenPR(ASTKind::LEAF_INT) {}

bool IntMatcher::match(Lexer &lexer) {
  auto token = lexer.peek(0);
  return token->getKind() == TokenKind::TK_INT;
}

//string
StrMatcher::StrMatcher(): MatchTokenPR(ASTKind::LEAF_Id) {}

bool StrMatcher::match(Lexer &lexer) {
  auto token = lexer.peek(0);
  return token->getKind() == TokenKind::TK_STR;
}
