#include "parse_order_tree.h"
#include "preprocessor.h"
#include "../env.h"

#include <string>

int main() {
  std::map<std::string, EnvPtr> environments;
  std::string entryFile("main.spr");

  Preprocessor p;
  auto tree = p.generateParsingOrder(environments, entryFile);

  if (environments.find("/home/zhu/wp/sparrow/build/pre_process/main.spr") == environments.end())
    std::cout << "failed 1" << std::endl;

  EnvPtr m = environments["/home/zhu/wp/sparrow/build/pre_process/main.spr"];
  auto foo = m->get("srcfoo");
  if (foo == nullptr)
    std::cout << "failed2" << std::endl;

  std::cout << "global env size: " << environments.size() << std::endl;
}
