#ifndef AST_H
#define AST_H
#include <unordered_map>

#include <utility>

#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"

class AST;
class Expr;
class Factor;
class BinaryOp;
class WithDecl;
class AssignmentExpr;
class RootNode;

class ASTVisitor {
public:
  virtual void visit(AST & /*unused*/){};
  virtual void visit(Expr & /*unused*/){};
  virtual void visit(Factor &) = 0;
  virtual void visit(BinaryOp &) = 0;
  virtual void visit(WithDecl &) = 0;
  virtual void visit(AssignmentExpr &) = 0;
  virtual void visit(RootNode &Node) = 0;
  virtual void visit(WriteNode &node) = 0;
};

class AST {
public:
  virtual ~AST() = default;
  virtual void accept(ASTVisitor &V) = 0;
};

class Expr : public AST {
public:
  Expr() = default;
};

class Factor : public Expr {
public:
  enum ValueKind { Ident, Number,int_ident};

private:
  ValueKind Kind;
  llvm::StringRef Val;

public:
  Factor(ValueKind Kind, llvm::StringRef Val) : Kind(Kind), Val(Val) {}
  ValueKind getKind() { return Kind; }
  llvm::StringRef getVal() { return Val; }
  void accept(ASTVisitor &V) override { V.visit(*this); }
};

class BinaryOp : public Expr {
public:
  enum Operator { Plus, Minus, Multiply, Divide };

private:
  Expr *Left;
  Expr *Right;
  Operator Op;

public:
  BinaryOp(Operator Op, Expr *L, Expr *R) : Op(Op), Left(L), Right(R) {}
  Expr *getLeft() { return Left; }
  Expr *getRight() { return Right; }
  Operator getOperator() { return Op; }
  void accept(ASTVisitor &V) override { V.visit(*this); }
};

class WithDecl : public AST {
  using VarVector = llvm::SmallVector<llvm::StringRef, 8>;
  VarVector Vars;//SmallVector 类型

public:
  WithDecl(llvm::SmallVector<llvm::StringRef, 8> Vars)
      : Vars(std::move(Vars)) {}
  VarVector::const_iterator begin() { return Vars.begin(); }
  VarVector::const_iterator end() { return Vars.end(); }
  void accept(ASTVisitor &V) override { V.visit(*this); }
};

// 在 AST.h 中添加新的节点类
class AssignmentExpr : public Expr {
public:
    AssignmentExpr(llvm::StringRef Var, Expr *Value)
        : Variable(Var), AssignedValue(Value) {}

    llvm::StringRef getVariable() const { return Variable; }
    Expr *getAssignedValue() const { return AssignedValue; }

    void accept(ASTVisitor &V) override { V.visit(*this); }

private:
    llvm::StringRef Variable;
    Expr *AssignedValue;
};

class RootNode : public AST {
public:
    RootNode(WithDecl *withNode, llvm::SmallVector<AssignmentExpr *, 8> intDecls, Expr *mainExprNode)
        : WithNode(withNode), IntDecls(intDecls), MainExprNode(mainExprNode) {}
        

    WithDecl *getWithNode() const { return WithNode; }
    llvm::SmallVector<AssignmentExpr *, 8> getIntDecls() const { return IntDecls; }
    Expr *getMainExprNode() const { return MainExprNode; }

    void accept(ASTVisitor &V) override { V.visit(*this); }

private:
    WithDecl *WithNode;
    llvm::SmallVector<AssignmentExpr *, 8> IntDecls;
    Expr *MainExprNode;
};

class WriteNode : public AST {
public:
    WriteNode(Expr *expression) : Expression(expression) {}

    Expr *getExpression() const { return Expression; }

    void accept(ASTVisitor &V) override {V.visit(*this);}

private:
    Expr *Expression;
};




#endif