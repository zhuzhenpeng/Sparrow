#include "fileIO.h"

#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>

#include "../debugger.h"


/************************打开只读文件***************************/

__OpenROFile::__OpenROFile(): NativeFunction("__OpenROFile", 1) {}

ObjectPtr __OpenROFile::invoke(const std::vector<ObjectPtr> &params) {
  if (params.empty() || params[0]->kind_ != ObjKind::STRING) {
    std::cerr << "Invalid params for __OpenROFile" << std::endl;
    return std::make_shared<IntObject>(-1);
  }
  
  StrObjectPtr fileName = std::dynamic_pointer_cast<StrObject>(params[0]);
  int fd = open(fileName->str_.c_str(), O_RDONLY);
  return std::make_shared<IntObject>(fd);
}


/***********************打开只写文件**************************/

__OpenWOFile::__OpenWOFile(): NativeFunction("__OpenWOFile", 2) {}

ObjectPtr __OpenWOFile::invoke(const std::vector<ObjectPtr> &params) {
  if (params.empty() || params[0]->kind_ != ObjKind::STRING || 
      params[1]->kind_ != ObjKind::INT) {
    std::cerr << "Invalid params for __OpenWOFile" << std::endl;
    return std::make_shared<IntObject>(-1);
  }
  
  StrObjectPtr fileName = std::dynamic_pointer_cast<StrObject>(params[0]);
  IntObjectPtr openFlag = std::dynamic_pointer_cast<IntObject>(params[1]);
  int fd;
  if (openFlag->value_ == 0)
    fd = open(fileName->str_.c_str(), O_WRONLY|O_TRUNC);
  else
    fd = open(fileName->str_.c_str(), O_WRONLY|O_APPEND);

  return std::make_shared<IntObject>(fd);
}

/*************************关闭文件****************************/

__CloseFile::__CloseFile(): NativeFunction("__CloseFile", 1) {}

ObjectPtr __CloseFile::invoke(const std::vector<ObjectPtr> &params) {
  if (params.empty() || params[0]->kind_ != ObjKind::INT) {
    std::cerr << "Invalid params for __CloseFile" << std::endl;
    return std::make_shared<IntObject>(-1);
  }

  IntObjectPtr fd = std::dynamic_pointer_cast<IntObject>(params[0]);
  return std::make_shared<IntObject>(close(fd->value_));
}


/********************从文件中读取一个字符********************/

__ReadChar::__ReadChar(): NativeFunction("__ReadChar", 1) {}

ObjectPtr __ReadChar::invoke(const std::vector<ObjectPtr> &params) {
  if (params.empty() || params[0]->kind_ != ObjKind::INT) {
    std::cerr << "Invalid params for __ReadChar" << std::endl;
    return std::make_shared<IntObject>(-1);
  }
  IntObjectPtr fd = std::dynamic_pointer_cast<IntObject>(params[0]);

  char buffer[2];
  ssize_t result = read(fd->value_, buffer, MAX_READ);
  buffer[result] = '\0';

  if (result == -1 || result == 0)
    return std::make_shared<NoneObject>();
  else
    return std::make_shared<StrObject>(buffer);
}

/*******************从文件中读取一个单词********************/

__ReadWord::__ReadWord(): NativeFunction("__ReadWord", 1) {}

ObjectPtr __ReadWord::invoke(const std::vector<ObjectPtr> &params) {
  if (params.empty() || params[0]->kind_ != ObjKind::INT) {
    std::cerr << "Invalid params for __ReadWord" << std::endl;
    return std::make_shared<IntObject>(-1);
  }

  IntObjectPtr fd = std::dynamic_pointer_cast<IntObject>(params[0]);

  std::string word;
  char buffer[1];
  buffer[0] = ' ';
  ssize_t result = 0;

  while (buffer[0] == ' ') {
    result = read(fd->value_, buffer, 1);
    if (result == -1 || result == 0)
      return std::make_shared<NoneObject>();
  }

  while (buffer[0] != ' ' && buffer[0] != '\n') {
    word.push_back(buffer[0]);
    result = read(fd->value_, buffer, 1);

  if (result == -1 || result == 0)
      break;
  }

  return std::make_shared<StrObject>(word);
}

/*****************从文件中读取一行字符*********************/

__ReadLine::__ReadLine(): NativeFunction("__ReadLine", 1) {}

ObjectPtr __ReadLine::invoke(const std::vector<ObjectPtr> &params) {
  if (params.empty() || params[0]->kind_ != ObjKind::INT) {
    std::cerr << "Invalid params for __ReadLine" << std::endl;
    return std::make_shared<IntObject>(-1);
  }

  IntObjectPtr fd = std::dynamic_pointer_cast<IntObject>(params[0]);
  std::string line;
  char buffer[1];

  ssize_t result = read(fd->value_, buffer, 1);
  if (result == -1 || result == 0)
    return std::make_shared<NoneObject>();

  while (buffer[0] != '\n') {
    line.push_back(buffer[0]);
    result = read(fd->value_, buffer, 1);

    if (result == -1 || result == 0)
      break;
  }

  //MyDebugger::print(line, __FILE__, __LINE__);

  return std::make_shared<StrObject>(line);
}

/************************向文件写入**********************/

__WriteFile::__WriteFile(): NativeFunction("__WriteFile", 2) {}

ObjectPtr __WriteFile::invoke(const std::vector<ObjectPtr> &params) {
  if (params.size() < 2 || params[0]->kind_ != ObjKind::INT || 
      params[1]->kind_ != ObjKind::STRING) {
    MyDebugger::print(static_cast<int>(params[0]->kind_), __FILE__, __LINE__);
    MyDebugger::print(static_cast<int>(params[1]->kind_), __FILE__, __LINE__);
    std::cerr << "Invalid params for __WriteFile" << std::endl;
    return std::make_shared<IntObject>(-1);   
  }

  IntObjectPtr fd = std::dynamic_pointer_cast<IntObject>(params[0]);
  StrObjectPtr str = std::dynamic_pointer_cast<StrObject>(params[1]);
  ssize_t result = write(fd->value_, str->str_.c_str(), str->str_.size());
  if (result == -1)
    return std::make_shared<IntObject>(-1);
  else
    return std::make_shared<IntObject>(0);
}
