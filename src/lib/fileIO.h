#ifndef SPARROW_FILEIO_H_
#define SPARROW_FILEIO_H_

/**
 * 文件IO的一些原生函数
 * 语言内部使用，前面统一加上__
 */


#include "../env.h"
#include "../native_func.h"

/************************打开只读文件***************************/

class __OpenROFile: public NativeFunction {
public:
  __OpenROFile();

  //入参：1个，文件名字
  //如果成功返回一个非负数int类型，代表文件描述符
  //否则返回-1 int类型
  ObjectPtr invoke(const std::vector<ObjectPtr> &params) override;
};

/***********************打开只写文件**************************/

class __OpenWOFile: public NativeFunction {
public:
  __OpenWOFile();

  //入参：1个，文件名字
  //如果成功返回一个非负数int类型，代表文件描述符
  //否则返回-1 int类型
  ObjectPtr invoke(const std::vector<ObjectPtr> &params) override;
};

/************************关闭文件****************************/

class __CloseFile: public NativeFunction {
public:
  __CloseFile();

  //入参：1个，文件描述符
  //返回0表示成功，-1表示失败
  ObjectPtr invoke(const std::vector<ObjectPtr> &params) override;
};

/********************从文件中读取一个字符********************/

class __ReadChar: public NativeFunction {
public:
  __ReadChar();

  //入参：1个，文件描述符
  //返回空表示已经读取到文件结尾，否则返回一个字符串类型
  ObjectPtr invoke(const std::vector<ObjectPtr> &params) override;

private:
  const size_t MAX_READ = 1;
};

/*******************从文件中读取一个单词********************/

class __ReadWord: public NativeFunction {
public:
  __ReadWord();

  //入参：1个，文件描述符
  //返回空表示已经读取到文件结尾，否则返回一个字符串类型
  ObjectPtr invoke(const std::vector<ObjectPtr> &params) override;
};

/*****************从文件中读取一行字符*********************/

class __ReadLine: public NativeFunction {
public:
  __ReadLine();

  //入参：1个，文件描述符、
  //返回空表示已经读取到文件结尾，否则返回一个字符串类型
  ObjectPtr invoke(const std::vector<ObjectPtr> &params) override;
};

/************************向文件写入**********************/

class __WriteFile: public NativeFunction {
public:
  __WriteFile();

  //入参：2个，文件描述符，写入内容
  //返回0表示写入成功，否则返回-1
  ObjectPtr invoke(const std::vector<ObjectPtr> &params) override;
};

#endif
