#include <iostream>
#include <memory>
#include "lexer_imp.h"

int main() {
  std::unique_ptr<Lexer> lexer(new LexerImp());
  lexer->parseFile("./src.txt");

  auto token = lexer->read();
  while (token->getKind() != TokenKind::TK_EOF) {
    std::cout << "token: " << static_cast<int>(token->getKind())
      << ", text: " << token->getText() 
      << ", line number: " << token->getLineNumber()
      << ", file name: " << token->getFileName() << std::endl;

    if (token->getKind() == TokenKind::TK_ID) {
      auto idTokenRawPtr = reinterpret_cast<IdToken*>(token.get());
      std::cout << "---" << idTokenRawPtr->getId() << std::endl;
    }
    token = lexer->read();
  }
}
