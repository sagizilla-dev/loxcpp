#pragma once
#include "config.hpp"
#include "common.hpp"
#include "token.hpp"
#include "ast.hpp"

class RuntimeError: public std::exception {
public:
    int line;
    std::string message;
    std::string errorMessage;
    static bool errorFound;
    RuntimeError(int line, std::string errorMessage): line(line), errorMessage(errorMessage) {
        errorFound = true;
        message = "Runtime error[" + std::to_string(line)+"] -> " + errorMessage;
    }
    const char* what() const noexcept override {
        return message.c_str();
    }
};
bool RuntimeError::errorFound = false;

class Interpreter: public ExprVisitor, public StmtVisitor {
public:
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
    std::any evaluate(Expr* expr) {
        return expr->accept(this);
    }
    void execute(Stmt* stmt) {
        stmt->accept(this);
    }
};