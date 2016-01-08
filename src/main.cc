#include <iostream>
#include <memory>
#include <exception>
#include "lexer_imp.h"
#include "parser.h"

int main() {
  try {
  std::unique_ptr<Lexer> lexer(new LexerImp());
  lexer->parseFile("./src.txt");

  BasicParser bp;
  bp.init();

  //auto token = lexer->read();
  //while (token->getKind() != TokenKind::TK_EOF) {
    //std::cout << "token: " << static_cast<int>(token->getKind())
      //<< ", text: " << token->getText() 
      //<< ", line number: " << token->getLineNumber()
      //<< ", file name: " << token->getFileName() << std::endl;

    //if (token->getKind() == TokenKind::TK_ID) {
      //auto idTokenRawPtr = reinterpret_cast<IdToken*>(token.get());
      //std::cout << "---" << idTokenRawPtr->getId() << std::endl;
    //}
    //token = lexer->read();
  //}
  
  auto tree = bp.parse(*lexer);
  } 
  catch (std::exception &e) {
    std::cout << e.what() << std::endl;
    return -1;  
  }
}
