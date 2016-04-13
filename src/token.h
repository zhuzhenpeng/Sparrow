#ifndef SPARROW_TOKEN_H_
#define SPARROW_TOKEN_H_

#include <string>
#include <memory>

//Token类型
enum class TokenKind {
  TK_ID = 1, 
  TK_INT = 2, 
  TK_FLOAT = 3,
  TK_STR = 4, 
  TK_EOF = -1,
  TK_EOL = -2
};

/*********************Token********************************/
//Token基类，每个Token都记录着自己的行数、文本内容、类型
class Token {
public:
  Token(int lineNumber, const std::string &fileName, const std::string &text);
  int getLineNumber() const;
  std::string getFileName() const;
  std::string getText() const;
  TokenKind getKind() const;

  //返回token类型的文字信息
  std::string kindInfo() const;

  //返回token的综合信息
  std::string info() const;

protected:
  int lineNumber_;
  std::string fileName_;
  std::string text_;
  TokenKind kind_;
};
using TokenPtr = std::shared_ptr<Token>;


/********************IntToken*****************************/
class IntToken: public Token {
public:
  IntToken(int lineNumber, const std::string &fileName, const std::string &text); 
  int getValue() const;
private:
  int value_;
};
using IntTokenPtr = std::shared_ptr<IntToken>;

/*******************FloatToken***************************/
class FloatToken: public Token {
public:
  FloatToken(int lineNumber, const std::string &fileName, const std::string &text);
  double getValue() const;
private:
  double value_;
};
using FloatTokenPtr = std::shared_ptr<FloatToken>;

/********************StrToken****************************/
class StrToken: public Token {
public:
  StrToken(int lineNumber, const std::string &fileName, const std::string &text);

private:
  //对源字符串进行处理，保留转义字符'\n'，识别\\和\"
  std::string polish(const std::string &srcStr);  
};
using StrTokenPtr = std::shared_ptr<StrToken>;


/********************IdToken****************************/
class IdToken: public Token {
public:
  IdToken(int lineNumber, const std::string &fileName, const std::string &text);
};
using IdTokenPtr = std::shared_ptr<IdToken>;


/********************EOFToken***************************/
class EOFToken;
using EOFTokenPtr = std::shared_ptr<EOFToken>;

class EOFToken: public Token {
public:
  static EOFTokenPtr getInstance();
private:
    EOFToken(int lineNumber, const std::string &fileName, const std::string &text);
private:
  static EOFTokenPtr eof_;
};

#endif
