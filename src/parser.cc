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
  auto arg_postfix = Parser::rule();

  //param
  auto param = Parser::rule()->id(reserved_);

  //params
  auto params = Parser::rule(ASTKind::LIST_PARAMETER)->commomPR(param)\
                ->repeatPR(Parser::rule()->custom(",", true)->commomPR(param));

  //param_list
  auto paramsList = Parser::rule()->custom("(", true)->optionPR(params)->custom(")", true);

  //primary
  auto primary = Parser::rule(ASTKind::LIST_PRIMARY_EXPR)->orPR({
        Parser::rule()->custom("(", true)->commomPR(expr)->custom(")", true),
        Parser::rule()->number(ASTKind::LEAF_INT),
        Parser::rule()->id(reserved_),
        Parser::rule()->str()
      })->repeatPR(arg_postfix);


  //factor
  auto factor = Parser::rule()->orPR({
        Parser::rule(ASTKind::LIST_NEGETIVE_EXPR)->custom("-", false)->commomPR(primary),
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
                                    Parser::rule()->custom("\\n", true)})\
                                ->optionPR(statement))\
               ->custom("}", true);

  //def
  auto def = Parser::rule(ASTKind::LIST_DEF_STMNT)->custom("def", true)->id(reserved_)\
             ->commomPR(paramsList)->commomPR(block);

  //args
  auto args = Parser::rule(ASTKind::LIST_ARGUMENTS)->commomPR(expr)\
              ->repeatPR(Parser::rule()->custom(",", true)->commomPR(expr));

  //arg_postfix
  arg_postfix->custom("(", true)->optionPR(args)->custom(")", true);

  //simple
  auto simple = Parser::rule(ASTKind::LIST_PRIMARY_EXPR)->commomPR(expr)->optionPR(args);

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
        def,
        Parser::rule(ASTKind::LIST_NULL_STMNT)
      })->orPR({
                Parser::rule()->custom(";", true), 
                Parser::rule()->custom("\\n", true)
              });
}

ASTreePtr BasicParser::parse(Lexer &lexer) {
    return program_->parse(lexer);
}

void BasicParser::initReserved() {
  //符号不用添加进来，符号被作为ID处理
  reserved_.insert(";");
  reserved_.insert("}");    
  reserved_.insert(")");
  reserved_.insert("\\n");
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
