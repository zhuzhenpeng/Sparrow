#include <gtest/gtest.h>

#include <iostream>
#include <memory>
#include <exception>
#include "../lexer_imp.h"
#include "../parser.h"

class EvalTest: public testing::Test {
protected:
  std::unique_ptr<Lexer> lexer;
  std::unique_ptr<BasicParser> parser;
  EnvPtr env;
  std::vector<int> expectResults;

  void SetUp() override{
    lexer.reset(new LexerImp());
    parser.reset(new BasicParser());
    parser->init();
    env.reset(new CommonEnv());

    expectResults = {0, 0, 1, 10, 45, 51};
  }

};

TEST_F(EvalTest, normal) {
  try {
    size_t resultIndex = 0;

    lexer->parseFile("./testing_files/eval_test.src");
    while (lexer->peek(0)->getKind() != TokenKind::TK_EOF) {
      auto tree = parser->parse(*lexer);

      if (tree != nullptr) {
        auto result = tree->eval(env);
        EXPECT_EQ(expectResults[resultIndex], 
                  std::static_pointer_cast<IntObject>(result)->value_);
        ++resultIndex;
      }
    }
  }
  catch (std::exception &e) {
    FAIL() << e.what();
  }
}



