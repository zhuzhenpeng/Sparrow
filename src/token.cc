#include "token.h"

Token::Token(int lineNumber): lineNumber_(lineNumber) {}

int Token::getLineNumber() {
  return lineNumber_;
}

std::string Token::getText() {
  return text_;
}

TokenKind Token::getKind() {
  return kind_;
}

