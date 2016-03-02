#include "preprocessor.h"

#include <queue>
#include <fstream>
#include "preprocess_exception.h"

Preprocessor::Preprocessor() {
  std::string requirePattern = "\\s*"
    "("
    "(require ([[:alnum:]_]+(\\.[[:alnum:]_]+)*) as ([[:alnum:]_]+))"
    "|(//.*)"
    ")?";

  try {
    requireRegex_ = std::regex(requirePattern, std::regex::optimize);
  }
  catch (std::regex_error e) {
    std::cerr << "regex error: " << e.std::exception::what() << 
      ", code: " << e.code() << std::endl;
    exit(-1);
  }
}

ParseOrderTreePtr Preprocessor::generateParsingOrder(std::map<std::string, EnvPtr> &env, 
      const std::string &entryFile) {
  
  //获取程序入口的绝对路径
  char buffer[200];
  char *absolutePath = realpath(entryFile.c_str(), buffer);
  if (absolutePath == nullptr) {
    throw PreprocessException("entry error, cannot find this file");
  }
  std::string entryAbPath(absolutePath);
  
  //初始化解析顺序树
  ParseOrderTreePtr parseOrderTree = std::make_shared<ParseOrderTree>(entryAbPath);
  DFSRequire(env, entryAbPath, parseOrderTree);

  return parseOrderTree;
}

void Preprocessor::DFSRequire(std::map<std::string, EnvPtr> &env, 
    const std::string &unitAbPath, ParseOrderTreePtr parseOrderTree) {
  
  std::ifstream file(unitAbPath);
  if (!file.is_open())
    throw PreprocessException("cannot find the unit: " + unitAbPath);

  std::string currUnitAbPathPrefix = getPathPrefix(unitAbPath);

  //如果当前unit还未被引用或解析过，那么为它创建环境(只发生在程序入口所在的unit)
  if (env.find(unitAbPath) == env.end())
    env[unitAbPath] = std::make_shared<CommonEnv>();
  EnvPtr currEnv = env[unitAbPath];

  RequireUnit requireUnit;  //引用unit
  while (parseRequire(file, requireUnit)) {
    std::string requireAbPath = currUnitAbPathPrefix + requireUnit.relativePath;
    //如果引用的unit还未被引用或解析过，那么为它创建环境
    if (env.find(requireAbPath) == env.end())
      env[requireAbPath] = std::make_shared<CommonEnv>();

    //把引用unit放入当前环境中
    currEnv->put(requireUnit.alias, env[requireAbPath]);

    //如果解析顺序树没有引用unit，则放入其中，并深度递归解析
    if (!parseOrderTree->hasNode(requireAbPath)) {
      parseOrderTree->putNew(unitAbPath, requireAbPath);
      DFSRequire(env, requireAbPath, parseOrderTree);
    }
  }
  file.close();
}

bool Preprocessor::parseRequire(std::istream &is, RequireUnit &unit) {
  std::string line;

READ_NEW_LINE:
  if (is)
    std::getline(is, line);
  else
    return false;
  std::smatch results;
  
  if (std::regex_search(line, results, requireRegex_)) {
    if (!results[1].matched || results[6].matched) {  //匹配到空格或注释，则继续读取下一行
      goto READ_NEW_LINE; 
    }
    else {
      unit.relativePath = formatRequireForm(results[3].str());
      unit.alias = results[5].str();
      return true;
    }
  }
  else {
    return false;
  }
}

std::string Preprocessor::formatRequireForm(const std::string &dotForm) {
  std::string result;
  size_t left = 0;
  size_t right;
  while ((right = dotForm.find(".", left)) != std::string::npos) {
    result += dotForm.substr(left, (right - left));
    result += "/";
    left = right + 1;
  }
  result += dotForm.substr(left);
  result += ".spr";
  return result;
}

std::string Preprocessor::getPathPrefix(const std::string &path) {
  if (path.empty())
    throw PreprocessException("get path prefix exception: empty path");

  size_t right = path.rfind("/");
  if (right == std::string::npos)
    throw PreprocessException("get path prefix exception: not found '/'");
  return path.substr(0, right + 1);
}
