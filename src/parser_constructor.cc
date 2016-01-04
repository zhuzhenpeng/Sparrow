#include "parser_constructor.h"

CommonParsePR::CommonParsePR(ParserPtr parser): parser_(parser) {}

void CommonParsePR::parse(Lexer &lexer, std::vector<ASTreePtr> &ast) {
  ast.push_back(parser_->parse(lexer));
}

bool CommonParsePR::match(Lexer &lexer) {
  return parser_->match(lexer);
}
