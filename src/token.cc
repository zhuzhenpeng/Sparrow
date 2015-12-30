#include "token.h"

/*********************Token********************************/

Token::Token(int lineNumber, const std::string &fileName, const std::string &text): 
  lineNumber_(lineNumber), fileName_(fileName), text_(text) {}

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

IntToken::IntToken(int lineNumber, const std::string &fileName, const std::string &text):
  Token(lineNumber, fileName, text) {
  value_ = std::stoi(text);
  kind_ = TokenKind::TK_INT;   
}

int IntToken::getValue() const {
  return value_;
}

/********************StrToken****************************/

StrToken::StrToken(int lineNumber, const std::string &fileName, const std::string &text):
  Token(lineNumber, fileName, text) {
  //TODO
  kind_ = TokenKind::TK_STR; 
}

std::string StrToken::getString() const {
  return content_;
}

std::string StrToken::polish(const std::string &srcStr) {
  if (srcStr.empty())
    return "";

  std::string result;
  size_t len = srcStr.length() - 1;
  for (size_t i = 0; i < len; ++i) {
    char c = srcStr[i];
    if (c == '\\' && i+1 < len) {
      char c2 = srcStr[i+1];
      if (c2 == '"' || c2 == '\\') {
        c = srcStr[++i];
      } else if (c2 == '\n') {
        ++i;
        c = '\n';
      }
    }
    result.push_back(c);
  }

  return result;
}

/********************IdToken****************************/

IdToken::IdToken(int lineNumber, const std::string &fileName, const std::string &text):
  Token(lineNumber, fileName, text) {
  id_ = text;
  kind_ = TokenKind::TK_STR;
}

std::string IdToken::getId() const {
  return id_;
}

/********************EOFToken***************************/

EOFTokenPtr EOFToken::eof_ = nullptr;

EOFToken::EOFToken(int lineNumber, const std::string &fileName, const std::string &text):
  Token(lineNumber, fileName, text) {
  kind_ = TokenKind::TK_EOF;
}

EOFTokenPtr EOFToken::getInstance() {
  if (eof_ == nullptr) {
    eof_.reset(new EOFToken(-1, "", ""));
  }
  return eof_;
}
