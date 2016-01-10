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
    
    Environment env;
    while (lexer->peek(0)->getKind() != TokenKind::TK_EOF) {
      auto tree = bp.parse(*lexer);
      //if (tree != nullptr)
        //std::cout << tree->info() << std::endl;     
      
      if (tree != nullptr) {
        auto result = tree->eval(env);
        switch (result->kind_) {
          case ObjKind::Bool:
            std::cout << std::static_pointer_cast<BoolObject>(result)->b_ << std::endl;
            break;
          case ObjKind::String:
            std::cout << std::static_pointer_cast<StrObject>(result)->str_ << std::endl;
            break;
          case ObjKind::Int:
            std::cout << std::static_pointer_cast<IntObject>(result)->value_ << std::endl;
            break;
        }
      }
    }
  } 
  catch (std::exception &e) {
    std::cout << e.what() << std::endl;
    return -1;  
  }
}
