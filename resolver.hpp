#pragma once
#include "config.hpp"
#include "common.hpp"
#include "token.hpp"
#include "ast.hpp"
#include "environment.hpp"
#include "callable.hpp"
#include "interpreter.hpp"

// this is to check if we are inside a function or not
enum FUNCTION_TYPE {
    NONE, FUNCTION
};
class Resolver: public ExprVisitor, public StmtVisitor {
public:
    Interpreter* interpreter;
    // this keeps track of scopes, similar to environments chain
    // we also store local scopes only, meaning that if we run out of scopes, it means the variable
    // is defined in the global scope for sure. If not, the interpret will throw a runtime error
    // key is the variable's name, value is its status, i.e if it has been resolved or not
    std::vector<std::unordered_map<std::string, bool>> scopes;
    FUNCTION_TYPE currentFunction;
    Resolver(Interpreter* interpreter): interpreter(interpreter), currentFunction(NONE) {

    }
    std::any visitBlockStmt(BlockStmt* stmt) override {
        beginScope();
        resolve(stmt->statements);
        endScope();
        return std::any{};
    }
    std::any visitVarDeclarationStmt(VarDeclarationStmt* stmt) override {
        // declaration is split from definition to handle the case
        // where a variable is used to initialize a variable with the same name, i.e
        // var a = a;
        // from the interpreter's perspective, it makes sense as we can resolve expression "a" if a
        // has been defined previously, but this is most likely a user's error
        declare(stmt->name);
        if (stmt->initializer) {
            resolve(stmt->initializer);
        }
        define(stmt->name);
        return std::any{};
    }
    std::any visitAssignExpr(AssignExpr* expr) override {
        resolve(expr->value);
        resolveLocal(expr, expr->name);
        return std::any{};
    }
    std::any visitFunDeclarationStmt(FunDeclarationStmt* stmt) override {
        declare(stmt->name);
        define(stmt->name);
        resolveFunction(stmt, FUNCTION_TYPE::FUNCTION);
        return std::any{};
    }
    std::any visitClassDeclarationStmt(ClassDeclarationStmt* stmt) override {
        declare(stmt->name);
        define(stmt->name);
        return std::any{};
    }
    void resolveFunction(FunDeclarationStmt* stmt, FUNCTION_TYPE type) {
        FUNCTION_TYPE enclosing = currentFunction;
        currentFunction = type;
        beginScope();
        for (Token param: stmt->parameters) {
            declare(param);
            define(param);
        }
        // we resolve all variables inside the function's body during DECLARATION, as opposed to
        // how the interpreter only executes function's body if the function has been called
        resolve(stmt->body);
        endScope();
        currentFunction = enclosing;
    }
    std::any visitExpressionStmt(ExpressionStmt* stmt) override {
        resolve(stmt->expr);
        return std::any{};
    }
    std::any visitIfStmt(IfStmt* stmt) override {
        resolve(stmt->condition);
        resolve(stmt->thenStatement);
        if (stmt->elseStatement) {
            resolve(stmt->elseStatement);
        }
        return std::any{};
    }
    std::any visitPrintStmt(PrintStmt* stmt) override {
        resolve(stmt->expr);
        return std::any{};
    }
    std::any visitReturnStmt(ReturnStmt* stmt) override {
        if (currentFunction!=FUNCTION) {
            std::cerr<<ResolverError(stmt->keyword.line, "Cannot return from top-level code").what()<<'\n';
        }
        if (stmt->value) {
            resolve(stmt->value);
        }
        return std::any{};
    }
    std::any visitWhileStmt(WhileStmt* stmt) override {
        resolve(stmt->condition);
        resolve(stmt->body);
        return std::any{};
    }
    std::any visitBinaryExpr(BinaryExpr* expr) override {
        resolve(expr->left);
        resolve(expr->right);
        return std::any{};
    }
    std::any visitCallExpr(CallExpr* expr) override {
        resolve(expr->callee);
        for (auto arg: expr->arguments) {
            resolve(arg);
        }
        return std::any{};
    }
    std::any visitGroupingExpr(GroupingExpr* expr) override {
        resolve(expr->expr);
        return std::any{};
    }
    std::any visitLiteralExpr(LiteralExpr* expr) override {
        return std::any{};
    }
    std::any visitLogicExpr(LogicExpr* expr) override {
        resolve(expr->left);
        resolve(expr->right);
        return std::any{};
    }
    std::any visitUnaryExpr(UnaryExpr* expr) override {
        resolve(expr->expr);
        return std::any{};
    }
    std::any visitVariableExpr(VariableExpr* expr) override {
        if (!scopes.empty() && scopes.back().count(expr->name.lexeme) && scopes.back()[expr->name.lexeme]==false) {
            std::cout<<ResolverError(expr->name.line, "Cannot read local variable in it's own initializer").what()<<'\n';
        }
        resolveLocal(expr, expr->name);
        return std::any{};
    }
    void resolveLocal(Expr* expr, Token name) {
        // start walking from the current environment outwards until we find
        // the closest declaration
        for (int i=scopes.size()-1; i>=0; i--) {
            if (scopes[i].count(name.lexeme)) {
                interpreter->resolve(expr, scopes.size()-1-i);
                return;
            }
        }
        // if the variable was never found, we assume it is inside the global scope
    }
    void declare(Token name) {
        // check if we are inside the global scope
        if (scopes.empty()) {
            return;
        }
        if (scopes.back().count(name.lexeme)) {
            std::cerr<<ResolverError(name.line, "Redefinition of a variable within a local scope is not allowed").what()<<'\n';
        }
        (scopes.back())[name.lexeme]=false; // the variable is declared in this scope, but hasn't been resolved
    }
    void define(Token name) {
        if (scopes.empty()) {
            return;
        }
        (scopes.back())[name.lexeme]=true;
    }
    void resolve(std::vector<Stmt*> statements) {
        for (auto stmt: statements) {
            resolve(stmt);
        }
    }
    void resolve(Stmt* stmt) {
        stmt->accept(this);
    }
    void resolve(Expr* expr) {
        expr->accept(this);
    }
    void beginScope() {
        scopes.push_back(std::unordered_map<std::string, bool>());
    }
    void endScope() {
        scopes.pop_back();
    }
};