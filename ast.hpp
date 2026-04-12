#pragma once
#include "config.hpp"
#include "common.hpp"
#include "token.hpp"
class BinaryExpr;
class UnaryExpr;
class GroupingExpr;
class LiteralExpr;
class VariableExpr;
class AssignExpr;
class LogicExpr;
class CallExpr;
class GetExpr;
class SetExpr;
class ThisExpr;
class ExprVisitor {
public:
	virtual std::any visitBinaryExpr(BinaryExpr* expr) = 0;
	virtual std::any visitUnaryExpr(UnaryExpr* expr) = 0;
	virtual std::any visitGroupingExpr(GroupingExpr* expr) = 0;
	virtual std::any visitLiteralExpr(LiteralExpr* expr) = 0;
	virtual std::any visitVariableExpr(VariableExpr* expr) = 0;
	virtual std::any visitAssignExpr(AssignExpr* expr) = 0;
	virtual std::any visitLogicExpr(LogicExpr* expr) = 0;
	virtual std::any visitCallExpr(CallExpr* expr) = 0;
	virtual std::any visitGetExpr(GetExpr* expr) = 0;
	virtual std::any visitSetExpr(SetExpr* expr) = 0;
	virtual std::any visitThisExpr(ThisExpr* expr) = 0;
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
class VariableExpr: public Expr{
public:
	VariableExpr(Token name): name(name) {
	}
	Token name;
	std::any accept(ExprVisitor* visitor) override {
		return visitor->visitVariableExpr(this);
	}
};
class AssignExpr: public Expr{
public:
	AssignExpr(Token name, Expr* value): name(name), value(value) {
	}
	Token name;
	Expr* value;
	std::any accept(ExprVisitor* visitor) override {
		return visitor->visitAssignExpr(this);
	}
};
class LogicExpr: public Expr{
public:
	LogicExpr(Expr* left, Expr* right, Token op): left(left), right(right), op(op) {
	}
	Expr* left;
	Expr* right;
	Token op;
	std::any accept(ExprVisitor* visitor) override {
		return visitor->visitLogicExpr(this);
	}
};
class CallExpr: public Expr{
public:
	CallExpr(Expr* callee, Token paren, std::vector<Expr*> arguments): callee(callee), paren(paren), arguments(arguments) {
	}
	Expr* callee;
	Token paren;
	std::vector<Expr*> arguments;
	std::any accept(ExprVisitor* visitor) override {
		return visitor->visitCallExpr(this);
	}
};
class GetExpr: public Expr{
public:
	GetExpr(Expr* object, Token name): object(object), name(name) {
	}
	Expr* object;
	Token name;
	std::any accept(ExprVisitor* visitor) override {
		return visitor->visitGetExpr(this);
	}
};
class SetExpr: public Expr{
public:
	SetExpr(Expr* object, Token name, Expr* value): object(object), name(name), value(value) {
	}
	Expr* object;
	Token name;
	Expr* value;
	std::any accept(ExprVisitor* visitor) override {
		return visitor->visitSetExpr(this);
	}
};
class ThisExpr: public Expr{
public:
	ThisExpr(Token keyword): keyword(keyword) {
	}
	Token keyword;
	std::any accept(ExprVisitor* visitor) override {
		return visitor->visitThisExpr(this);
	}
};
class PrintStmt;
class ExpressionStmt;
class VarDeclarationStmt;
class FunDeclarationStmt;
class ClassDeclarationStmt;
class WhileStmt;
class BlockStmt;
class IfStmt;
class ReturnStmt;
class StmtVisitor {
public:
	virtual std::any visitPrintStmt(PrintStmt* expr) = 0;
	virtual std::any visitExpressionStmt(ExpressionStmt* expr) = 0;
	virtual std::any visitVarDeclarationStmt(VarDeclarationStmt* expr) = 0;
	virtual std::any visitFunDeclarationStmt(FunDeclarationStmt* expr) = 0;
	virtual std::any visitClassDeclarationStmt(ClassDeclarationStmt* expr) = 0;
	virtual std::any visitWhileStmt(WhileStmt* expr) = 0;
	virtual std::any visitBlockStmt(BlockStmt* expr) = 0;
	virtual std::any visitIfStmt(IfStmt* expr) = 0;
	virtual std::any visitReturnStmt(ReturnStmt* expr) = 0;
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
class VarDeclarationStmt: public Stmt{
public:
	VarDeclarationStmt(Token name, Expr* initializer): name(name), initializer(initializer) {
	}
	Token name;
	Expr* initializer;
	std::any accept(StmtVisitor* visitor) override {
		return visitor->visitVarDeclarationStmt(this);
	}
};
class FunDeclarationStmt: public Stmt{
public:
	FunDeclarationStmt(Token name, std::vector<Token> parameters, std::vector<Stmt*> body): name(name), parameters(parameters), body(body) {
	}
	Token name;
	std::vector<Token> parameters;
	std::vector<Stmt*> body;
	std::any accept(StmtVisitor* visitor) override {
		return visitor->visitFunDeclarationStmt(this);
	}
};
class ClassDeclarationStmt: public Stmt{
public:
	ClassDeclarationStmt(Token name, std::vector<FunDeclarationStmt*> methods): name(name), methods(methods) {
	}
	Token name;
	std::vector<FunDeclarationStmt*> methods;
	std::any accept(StmtVisitor* visitor) override {
		return visitor->visitClassDeclarationStmt(this);
	}
};
class WhileStmt: public Stmt{
public:
	WhileStmt(Expr* condition, Stmt* body): condition(condition), body(body) {
	}
	Expr* condition;
	Stmt* body;
	std::any accept(StmtVisitor* visitor) override {
		return visitor->visitWhileStmt(this);
	}
};
class BlockStmt: public Stmt{
public:
	BlockStmt(std::vector<Stmt*> statements): statements(statements) {
	}
	std::vector<Stmt*> statements;
	std::any accept(StmtVisitor* visitor) override {
		return visitor->visitBlockStmt(this);
	}
};
class IfStmt: public Stmt{
public:
	IfStmt(Expr* condition, Stmt* thenStatement, Stmt* elseStatement): condition(condition), thenStatement(thenStatement), elseStatement(elseStatement) {
	}
	Expr* condition;
	Stmt* thenStatement;
	Stmt* elseStatement;
	std::any accept(StmtVisitor* visitor) override {
		return visitor->visitIfStmt(this);
	}
};
class ReturnStmt: public Stmt{
public:
	ReturnStmt(Token keyword, Expr* value): keyword(keyword), value(value) {
	}
	Token keyword;
	Expr* value;
	std::any accept(StmtVisitor* visitor) override {
		return visitor->visitReturnStmt(this);
	}
};
