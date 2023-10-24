#include "Parser.h"
#include "CodeGen.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/raw_ostream.h"
//with a : int b : b = a + 3 : with m : a = b + a * 10 - m : a + 3


AST *Parser::parse() {
  AST *Res = parseCalc();
  expect(Token::eoi);
  return Res;
}

AST *Parser::parseCalc() {
  Expr *E;
  llvm::SmallVector<llvm::StringRef, 8> Vars;
  llvm::StringRef VariableName;
  Expr *AssignedValue;
  AssignmentExpr *AssignmentNode;
  llvm::SmallVector<AssignmentExpr *, 8> intDecls;


  while(!Tok.is(Token::eoi)){
    //识别到 with 的处理
    if (Tok.is(Token::KW_with)) {
      advance();
      Vars.push_back(Tok.getText());
      advance();

      while (Tok.is(Token::comma)) {
        advance();
        Vars.push_back(Tok.getText());
        advance();
      }

    }

    //识别到 int 的处理
    else if(Tok.is(Token::KW_int)){
      advance();//读取并将b设置为变量
      advance();//读取到冒号
      advance();//读取到表达式的第一个b

    //加入符号表(失败，得在后续加入)
    VariableName = Tok.getText();
    advance();//读取到等号

    advance();//读取表达式右边第一个
    //调用parseExpr()解析表达式  
    AssignedValue = parseExpr();
    // 创建赋值节点
    AssignmentExpr *AssignmentNode = new AssignmentExpr(VariableName, AssignedValue);     
    }  

    //二者都不是时候的处理(此时tok为字符或数字表达式)
    else{
      //判断是否是赋值表达式 如a = b + a * 10 - m
      if(Tok.is(Token::ident)){
        VariableName = Tok.getText();//先把变量记下来
        advance();//读取第二个字符
        if(Tok.is(Token::equals)){//读到等号，是赋值表达式
          advance();//读取等号右侧的第一个符号
          AssignedValue = parseExpr();//识别等号右边的表达式
          AssignmentNode = new AssignmentExpr(VariableName, AssignedValue);//传入变量名和表达式
          intDecls.push_back(AssignmentNode);//传入节点指针
        }
        else{//算数表达式
          E = parseExpr();
        }
      }
      else{//第一个标识符是数字，即算术表达式
        E = parseExpr();
      }
    }
    if (!Tok.is(Token::eoi)) {
      advance();    
    }
    else break;//读完结束

  }

  if (Vars.empty()){
    return new RootNode(nullptr, intDecls, E);
  }
  WithDecl *withNode = new WithDecl(Vars);
  RootNode *rootNode = new RootNode(withNode, intDecls, E);
  return rootNode;
  //return new WithDecl(Vars, E);

_error:
  while (Tok.getKind() != Token::eoi)
    advance();
  return nullptr;
}

Expr *Parser::parseExpr() {
  Expr *Left = parseTerm();

  while (Tok.isOneOf(Token::plus, Token::minus)) {
    BinaryOp::Operator Op =
        Tok.is(Token::plus) ? BinaryOp::Plus : BinaryOp::Minus;

    advance();

    Expr *Right = parseTerm();
    Left = new BinaryOp(Op, Left, Right);
  }

  return Left;
}


Expr *Parser::parseTerm() {
  Expr *Left = parsePrimary();

  while (Tok.isOneOf(Token::multiply, Token::divide)) {
    BinaryOp::Operator Op =
        Tok.is(Token::multiply) ? BinaryOp::Multiply : BinaryOp::Divide;
    advance();
    Expr *Right = parsePrimary();
    Left = new BinaryOp(Op, Left, Right);
  }

  return Left;
}

 Expr *Parser::parsePrimary() {
  Expr *Res = nullptr;

  switch (Tok.getKind()) {
    case Token::number:
      Res = new Factor(Factor::Number, Tok.getText());
      advance();
      break;
    case Token::ident:
      Res = new Factor(Factor::Ident, Tok.getText());
      advance();
      break;
    case Token::left_paren:
      advance(); // 跳过左括号
      Res = parseExpr(); // 递归解析括号内的表达式
      expect(Token::right_paren);
      advance(); // 跳过右括号
      break;
    case Token::int_ident:
      Res = new Factor(Factor::int_ident, Tok.getText());
      advance();
      break;
    default:
      if (!Res)
        error();
      while (!Tok.isOneOf(Token::plus, Token::minus, Token::multiply,
                          Token::divide, Token::eoi, Token::right_paren))
        advance();
  }

  return Res;
}
