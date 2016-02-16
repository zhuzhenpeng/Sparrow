#include <iostream>
#include <fstream>
#include <memory>
#include <map>
#include <string>
#include <exception>

#include "../lexer_imp.h"
#include "../parser.h"
#include "../native_func.h"
#include "../pre_process/preprocess_exception.h"
#include "../pre_process/preprocessor.h"
#include "../pre_process/parse_order_tree.h"

std::unique_ptr<Lexer> lexer;
std::unique_ptr<BasicParser> parser;

//后序遍历树并解析
void run(std::map<std::string, EnvPtr> &env, ParseOrderTreeNodePtr node) {
  if (node == nullptr)
    throw PreprocessException("fatal error, null node");

  for (auto child: node->children) {
    run(env, child);
  }

  if (env.find(node->absolutePath) == env.end())
    throw PreprocessException("fatal error, not found environment for " + node->absolutePath);

  EnvPtr currEnv = env[node->absolutePath];
  NativeFuncInitializer::initialize(currEnv);     //每个模块都初始化原生函数

  lexer->parseFile(node->absolutePath);
  while (lexer->peek(0)->getKind() != TokenKind::TK_EOF) {
    auto tree = parser->parse(*lexer);

    if (tree != nullptr) {
      //std::cout << "[" << static_cast<int>(tree->kind_) << "]" << std::endl;
      std::cout << tree->info() << std::endl;
    }

    if (tree != nullptr) {
      auto result = tree->eval(currEnv);
      if (result != nullptr && result->kind_ == ObjKind::INT) {
        std::cout << std::static_pointer_cast<IntObject>(result)->value_ << std::endl;
      }
    }
  }
}

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

    //词法解析器
    lexer.reset(new LexerImp());

    //语法解析器
    parser.reset(new BasicParser());
    parser->init();

    //预处理
    Preprocessor p;
    auto parseOrderTree = p.generateParsingOrder(environments, entryFile);

    run(environments, parseOrderTree->getRoot());   
  }
  catch (std::exception &e) {
    std::cerr << e.what() << std::endl;
  }
}
