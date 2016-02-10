#ifndef SPARROW_PREPROCESSOR_H_
#define SPARROW_PREPROCESSOR_H_

#include <string>
#include <map>
#include <stack>
#include <regex>
#include <iostream>
#include <exception>

#include "env.h"

//预处理器
//进行编译前的预处理
class Preprocessor {
private:
  struct RequireUnit {
    std::string relativePath;
    std::string alias;
  };

public:
  Preprocessor();

  void generateParsingOrder(std::map<std::string, EnvPtr> &env, 
      std::stack<std::string> &units, const std::string &entryFile);

private:
  bool parseRequire(std::istream &is, RequireUnit &unit);

private:
  std::regex requireRegex_;
};

class PreprocessException: public std::exception {
public:
  PreprocessException(const std::string &errMsg): errMsg_(errMsg) {}

  const char* what() const noexcept override {
    return errMsg_.c_str();
  }

private:
    std::string errMsg_;
};

#endif
