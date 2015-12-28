#ifndef SPARROW_TOKEN_H_
#define SPARROW_TOKEN_H_

#include <string>

//Token类型
enum class TokenKind {
  TK_ID, 
  TK_INT, 
  TK_STR, 
  TK_EOL, 
  TK_EOF
};


//Token基类，每个Token都记录着自己的行数、文本内容、类型
class Token {
public:
  Token(int lineNumber);
  int getLineNumber();
  std::string getText();
  TokenKind getKind();
private:
  int lineNumber_;
  std::string text_;
  TokenKind kind_;
};

#endif
