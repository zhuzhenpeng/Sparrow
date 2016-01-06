#include "parser_constructor.h"

#include "ast_leaf.h"
#include "ast_list.h"

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
    case ASTKind::LEAF_STR:
       result = std::make_shared<StrTokenAST>(token);
       break;
    case ASTKind::LEAF_COMMON:
       result = std::make_shared<ASTLeaf>(ASTKind::LEAF_COMMON, token);
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

MatchTokenPR::MatchTokenPR(ASTKind astKind): kind_(astKind) {};

void MatchTokenPR::parse(Lexer &lexer, std::vector<ASTreePtr> &ast) {
  if (match(lexer)) {
    //通过parse后，lexer消耗一个token
    ASTreePtr leaf = ASTFactory::getLeafInstance(kind_, lexer.read());
    ast.push_back(leaf);
  }
  else {
    std::string expect;
    switch (kind_) {
      case ASTKind::LEAF_INT:
        expect = "int";
        break;
      case ASTKind::LEAF_Id:
        expect = "id";
        break;
      case ASTKind::LEAF_STR:
        expect = "str";
        break;
      default:
        expect = "UNKNOWN";
        break;
    }
    throw NotMatchingException(lexer.peek(0), expect);
  }
}

/////////////////////id
IdMatcher::IdMatcher(std::set<std::string> &reserved):
  MatchTokenPR(ASTKind::LEAF_Id), reserved_(reserved) {}

bool IdMatcher::match(Lexer &lexer) {
  auto token = lexer.peek(0);
  return token->getKind() == TokenKind::TK_ID && 
    reserved_.find(reinterpret_cast<IdToken*>(token.get())->getId()) == reserved_.end();
}

/////////////////////int
IntMatcher::IntMatcher(): MatchTokenPR(ASTKind::LEAF_INT) {}

bool IntMatcher::match(Lexer &lexer) {
  auto token = lexer.peek(0);
  return token->getKind() == TokenKind::TK_INT;
}

/////////////////////string
StrMatcher::StrMatcher(): MatchTokenPR(ASTKind::LEAF_Id) {}

bool StrMatcher::match(Lexer &lexer) {
  auto token = lexer.peek(0);
  return token->getKind() == TokenKind::TK_STR;
}

/*******************************自定义终结符**********************************/

CustomTerminalSymbalPR::CustomTerminalSymbalPR(const std::string &pattern, bool skipFlag):
  pattern_(pattern), skipFlag_(skipFlag) {}

void CustomTerminalSymbalPR::parse(Lexer &lexer, std::vector<ASTreePtr> &ast) {
  if (match(lexer)) {
    if (!skipFlag_)
      ast.push_back(ASTFactory::getLeafInstance(ASTKind::LEAF_COMMON, lexer.read()));
  }
  else {
    throw NotMatchingException(lexer.peek(0), pattern_);
  }
}

bool CustomTerminalSymbalPR::match(Lexer &lexer) {
  auto token = lexer.peek(0);
  if (token->getKind() == TokenKind::TK_ID) {
    return reinterpret_cast<IdToken*>(token.get())->getId() == pattern_;
  }
  else {
    return false;
  }
}










/********************************Parser类*************************************/

ASTreePtr Parser::parse(Lexer &lexer) {
  return nullptr;
}

bool Parser::match(Lexer &lexer) {
  return false;
}



