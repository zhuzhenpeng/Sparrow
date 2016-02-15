#include "preprocessor.h"

#include <queue>
#include <set>
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
  
  //获取入口的绝对路径
  char buffer[200];
  char *absolutePath = realpath(entryFile.c_str(), buffer);
  if (absolutePath == nullptr) {
    throw PreprocessException("entry error, cannot find this file");
  }
  std::string entryAbPath(absolutePath);
  
  //初始化解析顺序树
  ParseOrderTreePtr parseOrderTree = std::make_shared<ParseOrderTree>(entryAbPath);

  std::queue<std::string> waitingUnits; //等待被解析的units的绝对路径
  std::set<std::string> parsedUnits;    ///已经被解析过的units

  waitingUnits.push(entryAbPath);
  while (!waitingUnits.empty()) {
    
    std::string currUnitAbPath = waitingUnits.front();
    waitingUnits.pop();
    std::string currUnitAbPathPrefix = getPathPrefix(currUnitAbPath);

    std::ifstream file(currUnitAbPath);
    if (!file.is_open())
      throw PreprocessException("cannot find the unit: " + currUnitAbPath);

    //如果当前unit还未被引用或解析过，那么为它创建环境
    if (env.find(currUnitAbPath) == env.end())
      env[currUnitAbPath] = std::make_shared<CommonEnv>();
    EnvPtr currEnv = env[currUnitAbPath];

    RequireUnit requireUnit;  //引用unit
    while (parseRequire(file, requireUnit)) {
      std::string requireAbPath = currUnitAbPathPrefix + requireUnit.relativePath;
      //如果引用的unit还未被引用或解析过，那么为它创建环境
      if (env.find(requireAbPath) == env.end())
        env[requireAbPath] = std::make_shared<CommonEnv>();
      
      //把引用unit放入当前环境中
      currEnv->put(requireUnit.alias, env[requireAbPath]);

      //如果解析顺序树没有引用unit，则放入其中
      if (!parseOrderTree->hasNode(requireAbPath))
        parseOrderTree->putNew(currUnitAbPath, requireAbPath);

      //如果引用unit未被解析过，则放入队列中
      if (parsedUnits.find(requireAbPath) == parsedUnits.end())
        waitingUnits.push(requireAbPath);

      parsedUnits.insert(currUnitAbPath);
    }
  }

  return parseOrderTree;
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
      std::cout << "source: " << results[3].str() << std::endl
        << "source alias: " << results[5].str() << std::endl;
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
