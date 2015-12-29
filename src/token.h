#ifndef SPARROW_TOKEN_H_
#define SPARROW_TOKEN_H_

#include <string>
#include <memory>

//Token类型
enum class TokenKind {
  TK_ID, 
  TK_INT, 
  TK_STR, 
  TK_EOF
};


/*********************Token********************************/
//Token基类，每个Token都记录着自己的行数、文本内容、类型
class Token {
public:
  Token(int lineNumber, const std::string &fileName);
  int getLineNumber() const;
  std::string getFileName() const;
  std::string getText() const;
  TokenKind getKind() const;

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
  IntToken(int lineNumber, const std::string &fileName, int value);
  int getNumber() const;
private:
  int value_;
};
using IntTokenPtr = std::shared_ptr<IntToken>;


/********************StrToken****************************/
class StrToken: public Token {
public:
  StrToken(int lineNumber, const std::string &fileName, std::string str);
  std::string getString() const;
private:
  std::string content_;
};
using StrTokenPtr = std::shared_ptr<StrToken>;


/********************IdToken****************************/
class IdToken: public Token {
public:
  IdToken(int lineNumber, const std::string &fileName, std::string id);
  std::string getId() const;
private:
  std::string id_;
};
using IdTokenPtr = std::shared_ptr<IdToken>;


/********************EOFToken***************************/
class EOFToken: public Token {
public:
  EOFToken();
};
using EOFTokenPtr = std::shared_ptr<EOFToken>;

extern EOFToken g_EOF_Token;

#endif
