#include "preprocessor.h"

Preprocessor::Preprocessor() {
  std::string requirePattern_ = "\\s*"
    "("
    "(require [[:alnum:]]+(\\.[[:alnum:]]+)* as [[:alnum:]]+)"
    "|(//.*)"
    ")?";

  try {
    requireRegex_ = std::regex(requirePattern_, std::regex::optimize);
  }
  catch (std::regex_error e) {
    std::cerr << "regex error: " << e.std::exception::what() << 
      ", code: " << e.code() << std::endl;
    exit(-1);
  }
}

void Preprocessor::generateParsingOrder(std::map<std::string, EnvPtr> &env, 
      std::stack<std::string> &units, const std::string &entryFile) {
  
  char buffer[200];
  char *absolutePath = realpath(entryFile.c_str(), buffer);
  if (absolutePath == nullptr) {
    throw PreprocessException("entry error, cannot find this file");
  }

  std::ifstream file(absolutePath);
  if (file.is_open()) {
     
  }
  file.close();
}

bool Preprocessor::parseRequire(std::istream &is, RequireUnit &unit) {
  std::string line;
  std::getline(is, line);
  std::smatch results;
  
  if (std::regex_search(line, results, requireRegex_)) {
    
  }
  else {
    return false;
  }
}
