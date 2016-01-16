#include <iostream>
#include <memory>
#include <exception>
#include "lexer_imp.h"
#include "parser.h"

int main() {
  try {
    std::unique_ptr<Lexer> lexer(new LexerImp());

    std::unique_ptr<BasicParser> parser(new BasicParser());
    parser->init();

    EnvPtr globalEnv = std::make_shared<Environment>();
    
    lexer->parseFile("./test.src");
    while (lexer->peek(0)->getKind() != TokenKind::TK_EOF) {
      auto tree = parser->parse(*lexer);

      if (tree != nullptr) {
        std::cout << tree->info() << std::endl;
      }

      if (tree != nullptr) {
        auto result = tree->eval(globalEnv);
        if (result != nullptr && result->kind_ == ObjKind::Int) {
          std::cout << std::static_pointer_cast<IntObject>(result)->value_ << std::endl;
        }
      }
    }
  }
  catch (std::exception &e) {
    std::cerr << e.what() << std::endl;
  }
}
