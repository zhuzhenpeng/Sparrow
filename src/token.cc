#include "token.h"

/*********************Token********************************/

Token::Token(int lineNumber, const std::string &fileName): 
  lineNumber_(lineNumber), fileName_(fileName) {}

int Token::getLineNumber() const {
  return lineNumber_;
}

std::string Token::getFileName() const {
  return fileName_;
}

std::string Token::getText() const {
  return text_;
}

TokenKind Token::getKind() const {
  return kind_;
}

/********************IntToken*****************************/

IntToken::IntToken(int lineNumber, const std::string &fileName, int value): 
  Token(lineNumber, fileName), value_(value) {
  kind_ = TokenKind::TK_INT;   
}

int IntToken::getNumber() const {
  return value_;
}

/********************StrToken****************************/

StrToken::StrToken(int lineNumber, const std::string &fileName, std::string str): 
  Token(lineNumber, fileName), content_(str) {
  kind_ = TokenKind::TK_STR; 
}

std::string StrToken::getString() const {
  return content_;
}

/********************IdToken****************************/

IdToken::IdToken(int lineNumber, const std::string &fileName, std::string id): 
  Token(lineNumber, fileName), id_(id) {
  kind_ = TokenKind::TK_STR;
}

std::string IdToken::getId() const {
  return id_;
}

/********************EOFToken***************************/

EOFToken::EOFToken(): Token(-1, "") {
  kind_ = TokenKind::TK_EOF;
}

EOFToken g_EOF_Token;
