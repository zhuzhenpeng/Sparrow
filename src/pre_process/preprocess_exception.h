#ifndef SPARROW_PREPROCESS_EXCEPTION_H_
#define SPARROW_PREPROCESS_EXCEPTION_H_

#include <exception>
#include <string>

/***************************预处理异常********************************/
class PreprocessException: public std::exception {
public:
  PreprocessException(const std::string &errMsg): errMsg_(errMsg) {}

  const char* what() const noexcept override {
    return errMsg_.c_str();
  }

private:
    std::string errMsg_;
};

/*************************解析顺序树异常*****************************/
class ParseOrderTreeException: public PreprocessException {
public:
  ParseOrderTreeException(const std::string &errMsg): PreprocessException(errMsg) {}
};


#endif
