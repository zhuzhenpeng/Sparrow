#include "debugger.h"

#include <iostream>

void MyDebugger::print(const std::string &msg, const std::string &file, int line) {
  std::cout << "file: " << file << " line: " << line << "| " << msg << std::endl;
}

void MyDebugger::print(const std::string &msg, const std::string &file) {
  std::cout << "file: " << file << "| " << msg << std::endl;
}

void MyDebugger::print(const std::string &msg, int line) {
  std::cout << "line: " << line << "| " << msg << std::endl;
}

void MyDebugger::print(const std::string &msg) {
  std::cout << msg << std::endl;
}

void MyDebugger::print(int msg, const std::string &file, int line) {
    std::cout << "file: " << file << " line: " << line << "| " << msg << std::endl;
}

void MyDebugger::print(int msg, const std::string &file) {
    std::cout << "file: " << file << "| " << msg << std::endl;
}

void MyDebugger::print(int msg, int line) {
    std::cout << "line: " << line << "| " << msg << std::endl;
}

void MyDebugger::print(int msg) {
    std::cout << msg << std::endl;
}

