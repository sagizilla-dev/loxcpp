#pragma once
#include "config.hpp"
#include "common.hpp"
#include "token.hpp"
class BinaryExpr;
class UnaryExpr;
class GroupingExpr;
class LiteralExpr;
class ExprVisitor {
public:
	virtual std::any visitBinaryExpr(BinaryExpr* expr) = 0;
	virtual std::any visitUnaryExpr(UnaryExpr* expr) = 0;
	virtual std::any visitGroupingExpr(GroupingExpr* expr) = 0;
	virtual std::any visitLiteralExpr(LiteralExpr* expr) = 0;
};
class Expr {
public:
	virtual std::any accept(ExprVisitor* visitor) = 0;
};
class BinaryExpr: public Expr{
public:
	BinaryExpr(Expr* left, Expr* right, Token op): left(left), right(right), op(op) {
	}
	Expr* left;
	Expr* right;
	Token op;
	std::any accept(ExprVisitor* visitor) override {
		return visitor->visitBinaryExpr(this);
	}
};
class UnaryExpr: public Expr{
public:
	UnaryExpr(Expr* expr, Token op): expr(expr), op(op) {
	}
	Expr* expr;
	Token op;
	std::any accept(ExprVisitor* visitor) override {
		return visitor->visitUnaryExpr(this);
	}
};
class GroupingExpr: public Expr{
public:
	GroupingExpr(Expr* expr): expr(expr) {
	}
	Expr* expr;
	std::any accept(ExprVisitor* visitor) override {
		return visitor->visitGroupingExpr(this);
	}
};
class LiteralExpr: public Expr{
public:
	LiteralExpr(std::any value): value(value) {
	}
	std::any value;
	std::any accept(ExprVisitor* visitor) override {
		return visitor->visitLiteralExpr(this);
	}
};
class PrintStmt;
class ExpressionStmt;
class StmtVisitor {
public:
	virtual std::any visitPrintStmt(PrintStmt* expr) = 0;
	virtual std::any visitExpressionStmt(ExpressionStmt* expr) = 0;
};
class Stmt {
public:
	virtual std::any accept(StmtVisitor* visitor) = 0;
};
class PrintStmt: public Stmt{
public:
	PrintStmt(Expr* expr): expr(expr) {
	}
	Expr* expr;
	std::any accept(StmtVisitor* visitor) override {
		return visitor->visitPrintStmt(this);
	}
};
class ExpressionStmt: public Stmt{
public:
	ExpressionStmt(Expr* expr): expr(expr) {
	}
	Expr* expr;
	std::any accept(StmtVisitor* visitor) override {
		return visitor->visitExpressionStmt(this);
	}
};
