#ifndef PARSER_H
#define PARSER_H

#include "AST.h"
#include "Lexer.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/raw_ostream.h"


class Parser {
  Lexer &Lex;
  Token Tok;
  bool HasError;

  void error() {
    llvm::errs() << "Unexpected: " << Tok.getText() << "\n";
    HasError = true;
  }

  void advance() { Lex.next(Tok); }

  bool expect(Token::TokenKind Kind) {
    if (!Tok.is(Kind)) {
      error();
      return true;
    }
    return false;
  }

//注意consume执行完后会advance()一次
  bool consume(Token::TokenKind Kind) {
    if (expect(Kind))
      return true;
    advance();
    return false;
  }

  AST *parseCalc();
  Expr *parseExpr();
  Expr *parseTerm();
  Expr *parseFactor();
  Expr *parsePrimary();  
  
public:
  explicit Parser(Lexer &Lex) : Lex(Lex), HasError(false) { advance(); }
  AST *parse();
  bool hasError() const { return HasError; }
};

#endif