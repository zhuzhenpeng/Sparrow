#include <iostream>
#include <memory>
#include "lexer_imp.h"

int main() {
  std::unique_ptr<Lexer> lexer(new LexerImp());
  lexer->parseFile("./src.txt");

  auto token = lexer->read();
  while (token->getKind() != TokenKind::TK_EOF) {
    std::cout << token->getText() << std::endl;
  }
}
