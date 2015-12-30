#include "lexer_imp.h"

#include <fstream>
#include <sstream>

LexerImp::LexerImp(): is_(nullptr) {
  std::string patternStr = \
  "\\s*((//.*)|([0-9]+)|(\"(\\\\\"|\\\\\\\\|\\\\n|[^\"])*\")"
  "|[A-Z_a-z][A-Z_a-z0-9]*|==|<=|>=|&&|\\|\\||[[:punct:]])?";

  try {
    parsePattern_ = std::regex(patternStr, std::regex::optimize);
  }
  catch (std::regex_error e) {
    std::cerr << "regex error: " << e.std::exception::what() << 
      ", code: " << e.code() << std::endl;
    exit(-1);
  }
}

LexerImp::~LexerImp() {
  resetStatus();
}

void LexerImp::parseText(const std::string &text) {
  resetStatus();
  srcText_ = text;
  is_ = new std::istringstream(text);
}

void LexerImp::parseFile(const std::string &fileName) {
  resetStatus();
  fileName_ = fileName;;
  std::ifstream *tmpIs = new std::ifstream(fileName);
  if (tmpIs->is_open()){
    is_ = tmpIs;
  }
  else {
    std::cerr << "open file: " << fileName << " failed" << std::endl;
    delete tmpIs;
  }
}

std::string LexerImp::getFileName() const {
  return fileName_;
}

//read总是返回队列最前端的Token，并把它出列
//如果队列为空，如果hasMore_仍为真，责尝试解析源的下一行；否则返回EOF Token
TokenPtr LexerImp::read() {
  if (fillQueue(0)) {
    auto ptr = tokenQueue_.front();
    tokenQueue_.pop_front();
    return ptr;
  } 
  else {
    return EOFToken::getInstance();
  }
}

//peek总是返回队列中第k+1个Token
//如果超出队列中的数量，则解析至足够的数量；如果仍然不够，则返回EOF Token
TokenPtr LexerImp::peek(int k) {
  if (fillQueue(k))
    return tokenQueue_[k];
  else
    return EOFToken::getInstance();
}

////////////////////////////////////// private methods

bool LexerImp::fillQueue(size_t i) {
  while (i >= tokenQueue_.size()) {
    if (hasMore_)
      parseNextLine();
    else
      return false;
  }
  return true;
}

void LexerImp::parseNextLine() {
  if (is_ == nullptr) {
    hasMore_ = false;
    return;
  }

  std::string line;
  std::getline(*is_, line);
  if (is_->eof()) {
    hasMore_ = false;
    return;
  }

  for (std::sregex_iterator it(line.begin(), line.end(), parsePattern_), endIt;
      it != endIt; ++it) {
    const std::smatch &results = *it;
    const std::string &matchStr = it->str();
    std::cout << "match: " << matchStr << std::endl;

    if (results[1].matched)    //空格
      continue;
    if (results[2].matched)   //注释
      continue;

    if (results[3].matched) { //匹配整数
      IntTokenPtr tp = std::make_shared<IntToken>(lineNumber_, 
          fileName_, matchStr);
      tokenQueue_.push_back(tp);
    } 
    else if (results[4].matched) { // 匹配字符串
      StrTokenPtr tp = std::make_shared<StrToken>(lineNumber_, 
          fileName_, matchStr);
      tokenQueue_.push_back(tp);
    }
    else {  //匹配ID
      IdTokenPtr tp = std::make_shared<IdToken>(lineNumber_, 
          fileName_, matchStr);
    }
  }

  ++lineNumber_;
}

void LexerImp::resetStatus() {
  tokenQueue_.clear();
  hasMore_ = true;
  lineNumber_ = 1;
  if (!fileName_.empty()) {
    std::ifstream *tmpIs = reinterpret_cast<std::ifstream *>(is_);
    tmpIs->close();
  } else {
    delete is_;
  }
  is_ = nullptr;
  srcText_.clear();
}
