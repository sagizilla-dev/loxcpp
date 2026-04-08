#pragma once
#include "config.hpp"
#include "common.hpp"
#include "token.hpp"
#include "ast.hpp"
#include "environment.hpp"

class Interpreter: public ExprVisitor, public StmtVisitor {
public:
    Environment* env; // current environment, starts off as global
    Interpreter() {
        env = new Environment;
    }
    void interpret(std::vector<Stmt*> statements) {
        try {
            for (auto stmt: statements) {
                execute(stmt);
            }
        } catch (const RuntimeError& e) {
            std::cerr<<e.what()<<'\n';
        }
    }
    std::string stringify(std::any value) {
        if (!value.has_value()) {
            return "nil";
        }
        if (value.type()==typeid(double)) {
            std::string str = std::to_string(std::any_cast<double>(value));
            // since all numbers are doubles including ints, we just trim off all zeroes
            // after the decimal
            if (std::floor(std::any_cast<double>(value))==std::any_cast<double>(value)) {
                str = std::to_string((long long)std::floor(std::any_cast<double>(value)));
            }
            return str;
        }
        if (value.type()==typeid(bool)) {
            std::string str = std::to_string(std::any_cast<bool>(value));
            return str;
        }
        if (value.type()==typeid(std::string)) {
            return std::any_cast<std::string>(value);
        }
        return ""; // unreachable
    }
    std::any visitBinaryExpr(BinaryExpr* expr) override {
        std::any left = evaluate(expr->left);
        std::any right = evaluate(expr->right);
        switch (expr->op.type) {
            case (MINUS): {
                checkNumberOperands(expr->op, left, right);
                return std::any_cast<double>(left)-std::any_cast<double>(right);
            }
            case (SLASH): {
                checkNumberOperands(expr->op, left, right);
                if (std::any_cast<double>(right)==0.0) {
                    throw RuntimeError(expr->op.line, "Division by zero");
                }
                return std::any_cast<double>(left)/std::any_cast<double>(right);
            }
            case (STAR): {
                checkNumberOperands(expr->op, left, right);
                return std::any_cast<double>(left)*std::any_cast<double>(right);
            }
            case (PLUS): {
                if (left.type()==typeid(double) && right.type()==typeid(double)) {
                    return std::any_cast<double>(left)+std::any_cast<double>(right);
                }
                if (left.type()==typeid(std::string) && right.type()==typeid(std::string)) {
                    return std::any_cast<std::string>(left)+std::any_cast<std::string>(right);
                }
                throw RuntimeError(expr->op.line, "Operands must be two numbers or strings");
            }
            case (GREATER): {
                checkNumberOperands(expr->op, left, right);
                return std::any_cast<double>(left)>std::any_cast<double>(right);
            }
            case (GREATER_EQUAL): {
                checkNumberOperands(expr->op, left, right);
                return std::any_cast<double>(left)>=std::any_cast<double>(right);
            }
            case (LESS): {
                checkNumberOperands(expr->op, left, right);
                return std::any_cast<double>(left)<std::any_cast<double>(right);
            }
            case (LESS_EQUAL): {
                checkNumberOperands(expr->op, left, right);
                return std::any_cast<double>(left)<=std::any_cast<double>(right);
            }
            case (BANG_EQUAL): {
                checkNumberOperands(expr->op, left, right);
                return !isEqual(left, right);
            }
            case (EQUAL_EQUAL): {
                checkNumberOperands(expr->op, left, right);
                return isEqual(left, right);
            }
        }
        return std::any{}; // unreachable
    }
    bool isEqual(std::any left, std::any right) {
        // two nils are equal
        if (!left.has_value() && !right.has_value()) {
            return true;
        }
        if (!left.has_value() || !right.has_value()) {
            return false;
        }
        if (left.type()==typeid(double) && right.type()==typeid(double)) {
            return std::any_cast<double>(left)==std::any_cast<double>(right);
        }
        if (left.type()==typeid(std::string) && right.type()==typeid(std::string)) {
            return std::any_cast<std::string>(left)==std::any_cast<std::string>(right);
        }
        return false; // unreachable
    }
    std::any visitGroupingExpr(GroupingExpr* expr) override {
        return evaluate(expr->expr);
    };
    std::any visitUnaryExpr(UnaryExpr* expr) override {
        std::any value = evaluate(expr->expr);
        switch (expr->op.type) {
            case (MINUS): {
                checkNumberOperand(expr->op, value);
                return -std::any_cast<double>(value);
            }
            case (BANG): {
                return !isTruthy(value);
            }
        }
        return std::any{}; // unreachable
    }
    std::any visitExpressionStmt(ExpressionStmt* stmt) override {
        evaluate(stmt->expr);
        return std::any{};
    }
    std::any visitPrintStmt(PrintStmt* stmt) override {
        std::any value = evaluate(stmt->expr);
        std::cout<<stringify(value)<<'\n';
        return std::any{};
    }
    void checkNumberOperands(Token op, std::any value1, std::any value2) {
        if (value1.type()!=typeid(double) || value2.type()!=typeid(double)) {
            throw RuntimeError(op.line, "Operands must be numbers");
        }
    }
    void checkNumberOperand(Token op, std::any value) {
        if (value.type()!=typeid(double)) {
            throw RuntimeError(op.line, "Operand must be a number");
        }
    }
    bool isTruthy(std::any value) {
        // only false and nil are false, the rest is true
        if (!value.has_value()) {
            return false;
        }
        if (value.type()==typeid(bool)) {
            return std::any_cast<bool>(value);
        }
        return true;
    }
    std::any visitLiteralExpr(LiteralExpr* expr) override {
        return expr->value;
    }
    std::any visitVarDeclarationStmt(VarDeclarationStmt* stmt) override {
        std::any value;
        if (stmt->initializer) {
            value = evaluate(stmt->initializer);
        }
        // if a variable has never been initiaized, we assign nil to it
        env->define(stmt->name.lexeme, value);
        return nullptr;
    }
    std::any visitVariableExpr(VariableExpr* expr) override {
        return env->get(expr->name);
    }
    std::any visitIfStmt(IfStmt* stmt) override {
        if (isTruthy(evaluate(stmt->condition))) {
            execute(stmt->thenStatement);
        } else if (stmt->elseStatement) {
            execute(stmt->elseStatement);
        }
        return std::any{};
    }
    std::any visitWhileStmt(WhileStmt* stmt) override {
        while (isTruthy(evaluate(stmt->condition))) {
            execute(stmt->body);
        }
        return std::any{};
    }
    std::any visitLogicExpr(LogicExpr* expr) override {
        std::any left = evaluate(expr->left);
        // short-circuit evaluation
        if (expr->op.type==OR) {
            if (isTruthy(left)) {
                // we return the result of the expression itself, so that
                // ("hello" or nil) returns "hello" instead of pure true
                return left;
            }
        } else {
            if (!isTruthy(left)) {
                return left;
            }
        }
        return evaluate(expr->right);
    }
    std::any visitAssignExpr(AssignExpr* expr) override {
        std::any value = evaluate(expr->value);
        env->assign(expr->name, value);
        return value;
    }
    std::any visitBlockStmt(BlockStmt* stmt) override {
        executeBlock(stmt->statements, new Environment(env));
        return std::any{};
    }
    void executeBlock(std::vector<Stmt*> statements, Environment* environment) {
        // save the previous environment to restore it once we exit the block
        Environment* previous = this->env;
        try {
            this->env = environment;
            for (auto stmt: statements) {
                execute(stmt);
            }
        } catch(const RuntimeError& e) {
            throw e;
        }
        this->env = previous;
    }
    std::any evaluate(Expr* expr) {
        return expr->accept(this);
    }
    void execute(Stmt* stmt) {
        stmt->accept(this);
    }
};