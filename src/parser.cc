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
  auto postfix = Parser::rule();
  auto block = Parser::rule(ASTKind::LIST_BLOCK_STMNT);
  auto elif = Parser::rule(ASTKind::LIST_ELIF_STMNT);
  auto condition = Parser::rule(ASTKind::LIST_CONDITION_STMNT);
  auto andLogic = Parser::rule(ASTKind::LIST_AND_LOGIC);
  auto orLogic = Parser::rule(ASTKind::LIST_OR_LOGIC);

  //param
  auto param = Parser::rule()->id(reserved_);

  //params
  auto params = Parser::rule()->orPR({
        Parser::rule(ASTKind::LIST_PARAMETER)->commomPR(param)\
          ->repeatPR(Parser::rule()->custom(",", true)->commomPR(param)),
        Parser::rule(ASTKind::LIST_PARAMETER)\
          ->commomPR(Parser::rule(ASTKind::LIST_NULL_STMNT))
      });

  //param_list
  auto paramsList = Parser::rule()->custom("(", true)->commomPR(params)->custom(")", true);

  //elements(array literal)
  auto elements = Parser::rule(ASTKind::LIST_ARRAY_LITERAL)->commomPR(expr)\
                  ->repeatPR(Parser::rule()->custom(",", true)->commomPR(expr));

  //primary
  auto primary = Parser::rule()\
      ->orPR(
          {Parser::rule(ASTKind::LIST_LAMB)\
            ->custom("lamb", true)->commomPR(paramsList)->commomPR(block),
          
          Parser::rule(ASTKind::LIST_PRIMARY_EXPR)->orPR({
            Parser::rule()->custom("[", true)->commomPR(elements)->custom("]", true),
            Parser::rule()->custom("(", true)->commomPR(expr)->custom(")", true),
            Parser::rule()->number(ASTKind::LEAF_INT),
            Parser::rule()->number(ASTKind::LEAF_FLOAT),
            Parser::rule()->id(reserved_),
            Parser::rule()->str()
            })->repeatPR(postfix)
          });


  //factor
  auto factor = Parser::rule()->orPR({
        Parser::rule(ASTKind::LIST_NEGETIVE_EXPR)->custom("-", false)->commomPR(primary),
        primary
      });

  //expr
  expr->binaryExpr(operators_, factor);

  //block
  block->custom("{", true)\
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
  auto args = Parser::rule()->orPR({
        Parser::rule(ASTKind::LIST_ARGUMENTS)->commomPR(expr)\
          ->repeatPR(Parser::rule()->custom(",", true)->commomPR(expr)),
        Parser::rule(ASTKind::LIST_ARGUMENTS)\
          ->commomPR(Parser::rule(ASTKind::LIST_NULL_STMNT))
      });

  //postfix
  postfix->orPR({
      Parser::rule(ASTKind::LIST_DOT)->custom(".", true)->id(reserved_),
      Parser::rule()->custom("(", true)->commomPR(args)->custom(")", true),
      Parser::rule(ASTKind::LIST_ARRAY_REF)->custom("[", true)->commomPR(expr)->custom("]", true)
      });


  //simple
  auto simple = Parser::rule(ASTKind::LIST_PRIMARY_EXPR)->commomPR(expr);

  //statement
  statement->orPR({
        //if
        Parser::rule(ASTKind::LIST_IF_STMNT)->custom("if", true)->commomPR(condition)\
        ->commomPR(block)->custom("\\n", true)->repeatPR(elif)->\
        optionPR(
            Parser::rule()->custom("else", true)->commomPR(block)
          ),
        
        //while
        Parser::rule(ASTKind::LIST_WHILE_STMNT)->custom("while", true)\
        ->commomPR(condition)->commomPR(block),
        
        //普通的算术表达式
        simple
      });

  //elif
  elif->custom("elif", true)->commomPR(condition)->commomPR(block)->custom("\\n", true);

  //condition
  condition->orPR({
        Parser::rule()->commomPR(expr),
        Parser::rule()->commomPR(andLogic),
        Parser::rule()->commomPR(orLogic)
      });

  //and_logic
  andLogic->custom("and", true)->custom("(", true)->commomPR(condition)\
    ->custom(",", true)->commomPR(condition)->custom(")", true);

  //or_logic
  orLogic->custom("or", true)->custom("(", true)->commomPR(condition)\
    ->custom(",", true)->commomPR(condition)->custom(")", true);

  //member
  auto member = Parser::rule()->orPR({def, simple});

  //class_body
  auto classBody = Parser::rule(ASTKind::LIST_CLASS_BODY)->custom("{", true)\
                   ->optionPR(member)\
                   ->repeatPR(
                        Parser::rule()->orPR({
                          Parser::rule()->custom(";", true), 
                          Parser::rule()->custom("\\n", true)})\
                        ->optionPR(member))\
                   ->custom("}", true);

  //def_class
  auto defClass = Parser::rule(ASTKind::LIST_CLASS_STMNT)->custom("class", true)\
                  ->id(reserved_)->optionPR(
                    Parser::rule()->custom("extends", true)->id(reserved_)    
                  )->commomPR(classBody);

  //program
  program_->orPR({
        defClass,
        def,
        statement,
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
  //以下符号不当作ID处理
  //运算符是当作ID处理的，不需要添加进来
  reserved_.insert(";");
  reserved_.insert("}");    
  reserved_.insert(")");
  reserved_.insert("]");
  reserved_.insert("\\n");
  reserved_.insert("and");
  reserved_.insert("or");
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
