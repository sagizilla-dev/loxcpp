#pragma once
#include "config.hpp"
#include "common.hpp"
#include "token.hpp"
#include "ast.hpp"

class ASTPrinter: public Visitor {
public:
    void print(Expr* expr) {
        expr->accept(this);
        std::cout<<'\n';
    }
    std::any visitBinaryExpr(BinaryExpr* expr) override {
        parenthesize(expr->op->lexeme, {expr->left, expr->right});
        return std::any{};
    }
    std::any visitGroupingExpr(GroupingExpr* expr) override {
        parenthesize("group", {expr->expr});
        return std::any{};
    };
    std::any visitUnaryExpr(UnaryExpr* expr) override {
        parenthesize(expr->op->lexeme, {expr->expr});
        return std::any{};
    }
    std::any visitLiteralExpr(LiteralExpr* expr) override {
        if (expr->value.has_value()) {
            if (expr->value.type()==typeid(std::string)) {
                std::cout<<std::any_cast<std::string>(expr->value);
            } else if (expr->value.type()==typeid(double)) {
                std::cout<<std::any_cast<double>(expr->value);
            } else if (expr->value.type()==typeid(int)) {
                std::cout<<std::any_cast<int>(expr->value);
            }
            return std::any{};
        }
        std::cout<<"nil";
        return std::any{};
    }
    void parenthesize(std::string op, std::vector<Expr*> exprs) {
        std::cout<<"("<<op;
        for (auto expr: exprs) {
            std::cout<<" ";
            expr->accept(this);
        }
        std::cout<<")";
    }
};