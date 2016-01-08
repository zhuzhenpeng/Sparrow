#include "parser.h"

#include <vector>
#include "ast_tree.h"

BasicParser::BasicParser() = default;

void BasicParser::init() {
  initReserved();
  initOperators();

  program_ = Parser::rule();
  auto expr = Parser::rule();
  auto statement = Parser::rule();

  //primary
  auto primary = Parser::rule(ASTKind::LIST_PRIMARY_EXPR)->orPR({
        Parser::rule()->custom("(", true)->commomPR(expr)->custom(")", true),
        Parser::rule()->number(ASTKind::LEAF_INT),
        Parser::rule()->id(reserved_),
        Parser::rule()->str()
      });


  //factor
  auto factor = Parser::rule()->orPR({
        Parser::rule()->custom("-", true)->commomPR(primary),
        primary
      });

  //expr
  expr->binaryExpr(operators_, factor);

  //block
  auto block = Parser::rule(ASTKind::LIST_BLOCK_STMNT)\
               ->custom("{", true)\
               ->optionPR(statement)\
               ->repeatPR(Parser::rule()\
                                ->orPR({
                                    Parser::rule()->custom(";", true),
                                    Parser::rule()->custom("\n", true)})\
                                ->optionPR(statement))\
               ->custom("}", true);

  //simple
  auto simple = Parser::rule(ASTKind::LIST_PRIMARY_EXPR)->commomPR(expr);

  //statement
  statement->orPR({
        //if
        Parser::rule(ASTKind::LIST_IF_STMNT)->custom("if", true)->commomPR(expr)\
        ->commomPR(block)->optionPR(
            Parser::rule()->custom("else", true)->commomPR(block)
          ),
        
        //while
        Parser::rule(ASTKind::LIST_WHILE_STMNT)->custom("while", true)\
        ->commomPR(expr)->commomPR(block),
        
        //普通的算术表达式
        simple
      });

  //program
  program_->orPR({
        statement,
        Parser::rule(ASTKind::LIST_NULL_STMNT)
      })->orPR({
                Parser::rule()->custom(";", true), 
                Parser::rule()->custom("\n", true)
              });
}

ASTreePtr BasicParser::parse(Lexer &lexer) {
  return program_->parse(lexer);
}

void BasicParser::initReserved() {
  reserved_.insert(";");
  reserved_.insert("}");
  reserved_.insert("\n");
}

void BasicParser::initOperators() {
  operators_.insert({"=", {1, Precedence::RIGHT}});
  operators_.insert({"==",{2, Precedence::LEFT}});
  operators_.insert({">", {2, Precedence::LEFT}});
  operators_.insert({"<", {2, Precedence::LEFT}});
  operators_.insert({"+", {3, Precedence::LEFT}});
  operators_.insert({"-", {3, Precedence::LEFT}});
  operators_.insert({"*", {4, Precedence::LEFT}});
  operators_.insert({"/", {4, Precedence::LEFT}});
  operators_.insert({"%", {4, Precedence::LEFT}});
}
