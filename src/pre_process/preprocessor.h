#ifndef SPARROW_PREPROCESSOR_H_
#define SPARROW_PREPROCESSOR_H_

#include <string>
#include <map>
#include <set>
#include <stack>
#include <regex>
#include <iostream>
#include <exception>

#include "../env.h"
#include "parse_order_tree.h"

/*预处理器
 * 生成解析顺序树
 * 初始化每个unit的环境
 */

/********************************预处理器*****************************/
//进行编译前的预处理
class Preprocessor {
private:
  struct RequireUnit {
    std::string relativePath; //相对路径名
    std::string alias;        //引用别名
  };

public:
  Preprocessor();

  //根据程序运行的入口文件，解析出解释运行的文件顺序
  ParseOrderTreePtr generateParsingOrder(std::map<std::string, EnvPtr> &env, 
      const std::string &entryFile);

private:
  //通过递归来深度遍历unit间的require
  //unitAbPath是解析模块的绝对路径
  void DFSRequire(std::map<std::string, EnvPtr> &env, const std::string &unitAbPath, 
      ParseOrderTreePtr parseOrderTree);

  //解析文件头部的导入命令，如果需要导入则返回真，导入信息放在出参unit中
  bool parseRequire(std::istream &is, RequireUnit &unit);

  //把xx.xx.xx格式转换成xx/xx/xx.spr格式
  std::string formatRequireForm(const std::string &dotForm);

  //根据文件的绝对路径，解析出它的前缀 eg:/a/b/c.spr -> /a/b/
  std::string getPathPrefix(const std::string &path);
private:
  std::regex requireRegex_;  // require xxx.xxx as yyy
};


#endif
