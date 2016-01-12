#ifndef SPARROW_DEBUGGER_H_
#define SPARROW_DEBUGGER_H_

#include <string>

class MyDebugger {
public:
  static void print(const std::string &msg, const std::string &file, int line);
  static void print(const std::string &msg, const std::string &file);
  static void print(const std::string &msg, int line);
  static void print(const std::string &msg);

  static void print(int msg, const std::string &file, int line);
  static void print(int msg, const std::string &file);
  static void print(int msg, int line);
  static void print(int msg);
};


#endif
