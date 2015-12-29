#include <iostream>
#include <string>
#include <sstream>

int main() {
  std::string s = "hello\nworld";
  std::istringstream is(s);

  std::string line;
  while (std::getline(is, line)) {
    std::cout << line << std::endl;
  }
}
