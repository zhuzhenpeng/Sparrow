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
    
    while (lexer->peek(0)->getKind() != TokenKind::TK_EOF) {
      auto tree = bp.parse(*lexer);
      if (tree != nullptr)
        std::cout << tree->info() << std::endl;     
    }
  } 
  catch (std::exception &e) {
    std::cout << e.what() << std::endl;
    return -1;  
  }
}
