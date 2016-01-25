#include "parser_constructor.h"

#include "ast_leaf.h"
#include "ast_list.h"
#include "debugger.h"

/***************************生成各类AST的静态工厂******************************/
ASTreePtr ASTFactory::getLeafInstance(ASTKind kind, TokenPtr token) {
  ASTreePtr result = nullptr;
  switch (kind) {
    case ASTKind::LEAF_Id:
       result = std::make_shared<IdTokenAST>(token);
       break;
    case ASTKind::LEAF_INT:
       result = std::make_shared<IntTokenAST>(token);
       break;
    case ASTKind::LEAF_STR:
       result = std::make_shared<StrTokenAST>(token);
       break;
    case ASTKind::LEAF_COMMON:
       result = std::make_shared<ASTLeaf>(ASTKind::LEAF_COMMON, token);
       break;
    default:
       throw ParseException("get null AST leaf instance: " + 
           std::to_string(static_cast<int>(kind)));
       break;
  }
  return result;
}

ASTreePtr ASTFactory::getListInstance(ASTKind kind) {
  ASTreePtr result = nullptr;
  switch (kind) {
    case ASTKind::LIST_COMMON:
      result = std::make_shared<ASTList>(ASTKind::LIST_COMMON, true);
      break;
    case ASTKind::LIST_PRIMARY_EXPR:
      result = std::make_shared<PrimaryExprAST>();
      break;  
    case ASTKind::LIST_NEGETIVE_EXPR:
      result = std::make_shared<NegativeExprAST>();
      break;
    case ASTKind::LIST_BINARY_EXPR:
      result = std::make_shared<BinaryExprAST>();
      break;
    case ASTKind::LIST_BLOCK_STMNT:
      result = std::make_shared<BlockStmntAST>();
      break;
    case ASTKind::LIST_IF_STMNT:
      result = std::make_shared<IfStmntAST>();
      break;
    case ASTKind::LIST_WHILE_STMNT:
      result = std::make_shared<WhileStmntAST>();
      break;
    case ASTKind::LIST_NULL_STMNT:
      result = std::make_shared<NullStmntAST>();
      break;
    case ASTKind::LIST_PARAMETER:
      result = std::make_shared<ParameterListAST>();
      break;
    case ASTKind::LIST_DEF_STMNT:
      result = std::make_shared<DefStmntAST>();
      break;
    case ASTKind::LIST_ARGUMENTS:
      result = std::make_shared<Arguments>();
      break;
    case ASTKind::LIST_LAMB:
      result = std::make_shared<LambAST>();
      break;
    case ASTKind::LIST_CLASS_BODY:
      result = std::make_shared<ClassBodyAST>();
      break;
    case ASTKind::LIST_CLASS_STMNT:
      result = std::make_shared<ClassStmntAST>();
      break;
    case ASTKind::LIST_INSTANCE_DOT:
      result = std::make_shared<InstanceDot>();
      break;
    default:
      throw ParseException("get null AST list instance");
      break;
  }
  return result;
}

/***************************commom*****************************************/

CommonParsePR::CommonParsePR(ParserPtr parser): parser_(parser) {}

void CommonParsePR::parse(Lexer &lexer, std::vector<ASTreePtr> &ast) {
  //std::cout << "common" << std::endl;
  auto subTree = parser_->parse(lexer);
  handleParseResult(subTree, ast);
}

bool CommonParsePR::match(Lexer &lexer) {
  return parser_->match(lexer);
}

/****************************or*******************************************/

OrParsePR::OrParsePR(const std::vector<ParserPtr> &parsers): parsers_(parsers) {}

void OrParsePR::parse(Lexer &lexer, std::vector<ASTreePtr> &ast) {
  //MyDebugger::print("or parse" + lexer.peek(0)->info(), __FILE__, __LINE__);
  ParserPtr parser = choosePs(lexer);
  if (parser == nullptr) {
    throw ParseException("or choose failed: " + lexer.peek(0)->info());
  }
  else {
    auto subTree = parser->parse(lexer);
    handleParseResult(subTree, ast);
  }
}

bool OrParsePR::match(Lexer &lexer) {
  return choosePs(lexer) != nullptr;
}

ParserPtr OrParsePR::choosePs(Lexer &lexer) {
  for (auto pptr: parsers_) {
    if (pptr->match(lexer))
      return pptr;
  }
  return nullptr;
}

/***************************repeat**************************************/

RepeatParsePR::RepeatParsePR(ParserPtr parser, bool onceFlag): 
  parser_(parser), onceFlag_(onceFlag) {}

void RepeatParsePR::parse(Lexer &lexer, std::vector<ASTreePtr> &ast) {
  //std::cout << "repeat parse" << std::endl;
  //至少是0次匹配
  while (match(lexer)) {
    auto subTree = parser_->parse(lexer);
    handleParseResult(subTree, ast);
    if (onceFlag_)
      break;
  } 
}

bool RepeatParsePR::match(Lexer &lexer) {
  //std::cout << "match? " << std::endl;
  return parser_->match(lexer);
}

/********************************匹配Token************************************/

MatchTokenPR::MatchTokenPR(ASTKind kind): kind_(kind) {};

void MatchTokenPR::parse(Lexer &lexer, std::vector<ASTreePtr> &ast) {
  //std::cout << "match token" << std::endl;
  if (match(lexer)) {
    //通过parse后，lexer消耗一个token
    //std::cout << "consume: " << lexer.peek(0)->getText() << std::endl;
    ASTreePtr leaf = ASTFactory::getLeafInstance(kind_, lexer.read());
    handleParseResult(leaf, ast);
  }
  else {
    std::string expect;
    switch (kind_) {
      case ASTKind::LEAF_INT:
        expect = "int";
        break;
      case ASTKind::LEAF_Id:
        expect = "id";
        break;
      case ASTKind::LEAF_STR:
        expect = "str";
        break;
      default:
        expect = "UNKNOWN";
        break;
    }
    throw NotMatchingException(lexer.peek(0), expect);
  }
}

/////////////////////id
IdMatcher::IdMatcher(std::set<std::string> &reserved):
  MatchTokenPR(ASTKind::LEAF_Id), reserved_(reserved) {}

bool IdMatcher::match(Lexer &lexer) {
  auto token = lexer.peek(0);
  
  //不在保留字中的才算是ID
  return token->getKind() == TokenKind::TK_ID && 
    reserved_.find(token->getText())== reserved_.end();
}

/////////////////////int
IntMatcher::IntMatcher(): MatchTokenPR(ASTKind::LEAF_INT) {}

bool IntMatcher::match(Lexer &lexer) {
  auto token = lexer.peek(0);
  return token->getKind() == TokenKind::TK_INT;
}

/////////////////////string
StrMatcher::StrMatcher(): MatchTokenPR(ASTKind::LEAF_STR) {}

bool StrMatcher::match(Lexer &lexer) {
  auto token = lexer.peek(0);
  return token->getKind() == TokenKind::TK_STR;
}

/*******************************自定义终结符**********************************/

CustomTerminalSymbalPR::CustomTerminalSymbalPR(const std::string &pattern, bool skipFlag):
  pattern_(pattern), skipFlag_(skipFlag) {}

void CustomTerminalSymbalPR::parse(Lexer &lexer, std::vector<ASTreePtr> &ast) {
  //std::cout << "custom parse " << pattern_ << std::endl;
  if (match(lexer)) {
    //std::cout << "consume: " << lexer.peek(0)->getText() << std::endl;
    if (!skipFlag_) {
      auto subTree = ASTFactory::getLeafInstance(ASTKind::LEAF_COMMON, lexer.read());
      handleParseResult(subTree, ast);
    }
    else {
      lexer.read(); //直接读取消耗掉token 
    }
  }
  else {
    throw NotMatchingException(lexer.peek(0), pattern_);
  }
}

bool CustomTerminalSymbalPR::match(Lexer &lexer) {
  auto token = lexer.peek(0);
  if (token->getKind() == TokenKind::TK_ID) {
    return token->getText() == pattern_;
  }
  else {
    return false;
  }
}

/******************************双目运算符*************************************/

bool Precedence::LEFT = true;
bool Precedence::RIGHT = false;

BinaryExprPR::BinaryExprPR(const std::map<std::string, Precedence> &operators, 
    ParserPtr parser): operators_(operators), parser_(parser) {}

void BinaryExprPR::parse(Lexer &lexer, std::vector<ASTreePtr> &ast) {
  //std::cout << "binary expr parse" << std::endl;
  ASTreePtr right = parser_->parse(lexer);
  Precedence prec = nextOperatorPrec(lexer);
  while (prec.weight_ != -1) {
    //下一个是运算符，需要构造一颗相应的二叉树，把上面解析出来的AST作为它的左节点
    right = constructBinaryTree(right, prec, lexer);
    prec = nextOperatorPrec(lexer);
  }
  handleParseResult(right, ast);
}

bool BinaryExprPR::match(Lexer &lexer) {
  return parser_->match(lexer); 
}

Precedence BinaryExprPR::nextOperatorPrec(Lexer &lexer) {
  auto token = lexer.peek(0);
  //符号被lexer解析时是被当作ID的
  if (token->getKind() != TokenKind::TK_ID) {
    return Precedence();
  }
  else {
    const std::string &symbol = token->getText();
    auto prec = operators_.find(symbol);
    if (prec == operators_.end())
      return Precedence();
    else
      return prec->second;
  }
}

ASTreePtr BinaryExprPR::constructBinaryTree(ASTreePtr leftFactor, const Precedence &opPrec, 
    Lexer &lexer) {
  ASTreePtr result = ASTFactory::getListInstance(ASTKind::LIST_BINARY_EXPR); 
  auto tmptr = std::dynamic_pointer_cast<BinaryExprAST>(result);
  auto &exprTree = tmptr->children();

  exprTree.push_back(leftFactor);
  //std::cout << "consume: " << lexer.peek(0)->getText() << std::endl;
  exprTree.push_back(ASTFactory::getLeafInstance(ASTKind::LEAF_COMMON, lexer.read()));
  auto rightFactor = parser_->parse(lexer);

  //如果右侧的运算优先级高于左侧，则递归构造解析树
  Precedence nextPrec = nextOperatorPrec(lexer);
  while (nextPrec.weight_ != -1 && isRightHiger(opPrec, nextPrec)) {
    rightFactor = constructBinaryTree(rightFactor, nextPrec, lexer);
    nextPrec = nextOperatorPrec(lexer);
  }
  exprTree.push_back(rightFactor);

  return result;
}

bool BinaryExprPR::isRightHiger(const Precedence &left, const Precedence &right) {
  //如果下一个操作符是右结合性的，即使优先值相等也是右侧的优先级更高
  if (right.leftAssoc_)
    return left.weight_ < right.weight_;
  else
    return left.weight_ <= right.weight_;
}

/********************************Parser类*************************************/

Parser::Parser(ASTKind kind): kind_(kind) {}

ASTreePtr Parser::parse(Lexer &lexer) {
  //MyDebugger::print(static_cast<int>(kind_), __LINE__);

  //返回结果是根据Parser本身的类型决定的
  ASTreePtr result = ASTFactory::getListInstance(kind_);

  auto tmptr = std::dynamic_pointer_cast<ASTList>(result);
  auto &children = tmptr->children();

  for (auto rule: rulesCombination_)
    rule->parse(lexer, children);

  
  //剪枝
  if (children.empty() && tmptr->ignore())
    return nullptr;
  else if (children.size() == 1 && tmptr->ignore())
    return children[0];
  else
    return result;
}

bool Parser::match(Lexer &lexer) {
  //规则为空只可能发生在LIST_NULL_STMNT的情况下
  //此时认为是匹配成功的
  if (rulesCombination_.empty())
    return true;
  else
    return rulesCombination_[0]->match(lexer);
}

ParserPtr Parser::rule() {
  return std::make_shared<Parser>(ASTKind::LIST_COMMON); 
}

ParserPtr Parser::rule(ASTKind kind) {
  if (static_cast<int>(kind) < static_cast<int>(ASTKind::LIST_COMMON)) {
    throw ParseException("Cannot construct rule for AST Leaf"); 
  }
  return std::make_shared<Parser>(kind); 
}

ParserPtr Parser::number(ASTKind kind) {
  //TODO
  switch (kind) {
    case ASTKind::LEAF_INT:
      rulesCombination_.push_back(std::make_shared<IntMatcher>());
      break;
    default:
      throw ParseException("TODO exception of Parser::number");
      break;
  }
  return shared_from_this();
}

ParserPtr Parser::id(std::set<std::string> &reserved) {
  rulesCombination_.push_back(std::make_shared<IdMatcher>(reserved));
  return shared_from_this();
}

ParserPtr Parser::str() {
  rulesCombination_.push_back(std::make_shared<StrMatcher>());
  return shared_from_this();
}

ParserPtr Parser::custom(const std::string &pattern, bool skipFlag) {
  rulesCombination_.push_back(std::make_shared<CustomTerminalSymbalPR>(pattern, skipFlag));
  return shared_from_this();
}

ParserPtr Parser::binaryExpr(const std::map<std::string, Precedence> &operators, 
    ParserPtr factorPs) {
  rulesCombination_.push_back(std::make_shared<BinaryExprPR>(operators, factorPs));
  return shared_from_this();
}

ParserPtr Parser::commomPR(ParserPtr parser) {
  rulesCombination_.push_back(std::make_shared<CommonParsePR>(parser));
  return shared_from_this();
}

ParserPtr Parser::orPR(const std::vector<ParserPtr> &parsers) {
  rulesCombination_.push_back(std::make_shared<OrParsePR>(parsers));
  return shared_from_this();
}

ParserPtr Parser::optionPR(ParserPtr parser) {
  rulesCombination_.push_back(std::make_shared<RepeatParsePR>(parser, true));
  return shared_from_this();
}

ParserPtr Parser::repeatPR(ParserPtr parser) {
  rulesCombination_.push_back(std::make_shared<RepeatParsePR>(parser, false));
  return shared_from_this();
}
