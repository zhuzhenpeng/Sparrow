#include <iostream>
#include <fstream>
#include <memory>
#include <map>
#include <stack>
#include <string>
#include <exception>

#include <stdlib.h>

#include "../lexer_imp.h"
#include "../parser.h"
#include "../native_func.h"

int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cerr << "Please confirm program entry" << std::endl;
    exit(-1);
  }
  try {

    /****************************预处理**************************************/

    //每个unit的环境，unit以绝对路径名为标识
    std::map<std::string, EnvPtr> environments;  //<绝对路径名，环境>

    //程序入口
    std::string entryFile(argv[1]);

    std::unique_ptr<Lexer> lexer(new LexerImp());

    std::unique_ptr<BasicParser> parser(new BasicParser());
    parser->init();

    EnvPtr globalEnv = std::make_shared<CommonEnv>();
    NativeFuncInitializer::initialize(globalEnv);
    
    lexer->parseFile(entryFile);
    while (lexer->peek(0)->getKind() != TokenKind::TK_EOF) {
      auto tree = parser->parse(*lexer);

      if (tree != nullptr) {
        //std::cout << "[" << static_cast<int>(tree->kind_) << "]" << std::endl;
        std::cout << tree->info() << std::endl;
      }

      if (tree != nullptr) {
        auto result = tree->eval(globalEnv);
        if (result != nullptr && result->kind_ == ObjKind::INT) {
          std::cout << std::static_pointer_cast<IntObject>(result)->value_ << std::endl;
        }
      }
    }
  }
  catch (std::exception &e) {
    std::cerr << e.what() << std::endl;
  }
}
