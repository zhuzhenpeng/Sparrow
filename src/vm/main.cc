#include <iostream>
#include <fstream>
#include <memory>
#include <map>
#include <string>
#include <exception>

#include "../lexer_imp.h"
#include "../parser.h"
#include "../native_func.h"
#include "../lib/fileIO.h"
#include "../pre_process/preprocess_exception.h"
#include "../pre_process/preprocessor.h"
#include "../pre_process/parse_order_tree.h"
#include "vm.h"

std::unique_ptr<Lexer> lexer;
std::unique_ptr<BasicParser> parser;

//初始化原生函数
void initNativeFunc() {
  NativeFuncInitializer::addToInitList(std::make_shared<NativePrint>());
  NativeFuncInitializer::addToInitList(std::make_shared<NativePrintLine>());
  NativeFuncInitializer::addToInitList(std::make_shared<NativeReadInt>());
  NativeFuncInitializer::addToInitList(std::make_shared<NativeReadFloat>());
  NativeFuncInitializer::addToInitList(std::make_shared<NativeReadString>());

  NativeFuncInitializer::addToInitList(std::make_shared<__OpenROFile>());
  NativeFuncInitializer::addToInitList(std::make_shared<__OpenWOFile>());
  NativeFuncInitializer::addToInitList(std::make_shared<__CloseFile>());
  NativeFuncInitializer::addToInitList(std::make_shared<__ReadChar>());
  NativeFuncInitializer::addToInitList(std::make_shared<__ReadWord>());
  NativeFuncInitializer::addToInitList(std::make_shared<__ReadLine>());
  NativeFuncInitializer::addToInitList(std::make_shared<__WriteFile>());
}

///经过预处理环境中已有其它unit的信息，但是符号表没有，因此初始化符号表
//初始化每个unit的符号表和环境，为环境添加上一些常见变量
void init(EnvPtr env, SymbolsPtr symbols) {
  //环境现在里面只有其它环境对象，直接遍历一遍全部元素即可
  MapEnvPtr mEnv = std::dynamic_pointer_cast<MapEnv>(env);
  const std::map<std::string, ObjectPtr> &elements = mEnv->getElements();
  for (auto &e: elements) {
    symbols->getRuntimeIndex(e.first);
  }

  symbols->getRuntimeIndex("nil");
  env->put("nil", std::make_shared<NoneObject>());

  symbols->getRuntimeIndex("true");
  env->put("true", std::make_shared<BoolObject>(true));

  symbols->getRuntimeIndex("false");
  env->put("false", std::make_shared<BoolObject>(false));

  NativeFuncInitializer::initialize(env, symbols);
}

//后序遍历树并解析
void evalAndCompile(std::map<std::string, EnvPtr> &env, ParseOrderTreeNodePtr node) {
  if (node == nullptr)
    throw PreprocessException("fatal error, null node");

  for (auto child: node->children) {
    evalAndCompile(env, child);
  }

  if (env.find(node->absolutePath) == env.end())
    throw PreprocessException("fatal error, not found environment for " + node->absolutePath);

  //初始化符号表和环境
  EnvPtr currEnv = env[node->absolutePath];
  SymbolsPtr currSymbols = std::make_shared<Symbols>(nullptr, SymbolsKind::UNIT);
  currEnv->setUnitSymbols(currSymbols);
  init(currEnv, currSymbols);

  lexer->parseFile(node->absolutePath);
  while (lexer->peek(0)->getKind() != TokenKind::TK_EOF) {
    auto tree = parser->parse(*lexer);

    //打印树的信息
    if (tree != nullptr) {
      //std::cout << "[" << static_cast<int>(tree->kind_) << "]" << std::endl;
      //std::cout << tree->info() << std::endl;
    }

    //遍历运行、编译树
    if (tree != nullptr) {
      //预处理生成符号表
      tree->preProcess(currSymbols);

      //运行全局操作，对函数、类进行编译
      auto result = tree->eval(currEnv);
    }
  }
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cerr << "Please confirm program entry" << std::endl;
    exit(-1);
  }
  try {
    //每个unit的环境，unit以绝对路径名为标识
    std::map<std::string, EnvPtr> environments;  //<绝对路径名，环境>

    //程序入口
    std::string entryFile(argv[1]);

    //词法解析器
    lexer.reset(new LexerImp());

    //语法解析器
    parser.reset(new BasicParser());
    parser->init();

    //初始化原生函数
    initNativeFunc();

    //预处理，生成解析顺序树
    Preprocessor p;
    auto parseOrderTree = p.generateParsingOrder(environments, entryFile);

    //运行全局环境中的操作，对每个函数、类进行编译
    evalAndCompile(environments, parseOrderTree->getRoot());   

    //通过虚拟机，从main函数开始运行程序
    EnvPtr mainEnv = environments[parseOrderTree->getRoot()->absolutePath];
    ObjectPtr mainFuncObj = nullptr;
    try {
      mainFuncObj = mainEnv->get("main");
    }
    catch (EnvNotFoundException &e){
      //没有main函数入口，正常退出程序
      return 0;
    }


    try {
      if (mainFuncObj->kind_ != ObjKind::FUNCTION) {
        std::cerr << "Invalid main function entry" << std::endl;
        return -1;
      }
      FuncPtr mainFunc = std::dynamic_pointer_cast<FuncObject>(mainFuncObj);
      ByteCodeInterpreter interpreter(mainFunc);
      interpreter.run();
    }
    catch (std::exception &e) {
      std::cerr << e.what() << std::endl;
      return -1;
    }
  }
  catch (std::exception &e) {
    std::cerr << e.what() << std::endl;
    return -1;
  }
}
