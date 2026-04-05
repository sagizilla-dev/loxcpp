#pragma once
#include "config.hpp"
#include "token.hpp"
#include "ast.hpp"

class SyntaxError: public std::exception {
public:
    int line;
    std::string message;
    std::string errorMessage;
    static bool errorFound;
    SyntaxError(int line, std::string errorMessage): line(line), errorMessage(errorMessage) {
        errorFound = true;
        message = "Syntax error[" + std::to_string(line)+"] -> " + errorMessage;
    }
    const char* what() const noexcept override {
        return message.c_str();
    }
};
bool SyntaxError::errorFound = false;

struct Parser {
    std::vector<Token> tokens;
    int current = 0;
    Parser(std::vector<Token> tokens): tokens(tokens) {

    }
    Expr* parse() {
        try {
            return expression();
        } catch (const SyntaxError& e) {
            std::cerr<<e.what()<<'\n';
        }
        return nullptr;
    }
    Expr* expression() {
        return equality();
    }
    Expr* equality() {
        Expr* left = comparison();

        while (match({EQUAL_EQUAL, BANG_EQUAL})) {
            Token op = previous();
            Expr* right = comparison();
            // storing back into left introduces left-associativity
            left = new BinaryExpr(left, right, op);
        }

        return left;
    }
    Expr* comparison() {
        Expr* left = term();

        while (match({LESS, LESS_EQUAL, GREATER, GREATER_EQUAL})) {
            Token op = previous();
            Expr* right = term();
            left = new BinaryExpr(left, right, op);
        }

        return left;
    }
    Expr* term() {
        Expr* left = factor();

        while (match({PLUS, MINUS})) {
            Token op = previous();
            Expr* right = factor();
            left = new BinaryExpr(left, right, op);
        }

        return left;
    }
    Expr* factor() {
        Expr* left = unary();

        while (match({STAR, SLASH})) {
            Token op = previous();
            Expr* right = unary();
            left = new BinaryExpr(left, right, op);
        }

        return left;
    }
    Expr* unary() {
        if (match({MINUS, BANG})) {
            Token op = previous();
            Expr* right = unary();
            return new UnaryExpr(right, op);
        }

        return primary();
    }
    Expr* primary() {
        if (match({FALSE})) {
            return new LiteralExpr(false);
        }
        if (match({TRUE})) {
            return new LiteralExpr(true);
        }
        if (match({NIL})) {
            return new LiteralExpr(std::any{});
        }
        if (match({STRING, NUMBER})) {
            return new LiteralExpr(previous().literal);
        }
        if (match({LEFT_PAREN})) {
            Expr* expr = expression();
            consume(RIGHT_PAREN, "Expected ) after expression");
            return new GroupingExpr(expr);
        }
        throw SyntaxError(peek().line, "Expected expression");
    }
    Token consume(TOKEN_TYPE tokenType, std::string message) {
        if (!checkTokenType(tokenType)) {
            // entering panic mode, start stack unwinding
            if (peek().type==END) {
                message+=" at end";
            } else {
                message+=" at '"+peek().lexeme+"'";
            }
            throw SyntaxError(peek().line, message);
        }
        return advance();
    }
    void synchronize() {
        advance();
        // keep going through tokens until we get to a statement boundary
        // thus we report the original error, get rid of possibly useless cascaded errors
        // and now back to parsing
        while (!isAtEnd()) {
            if (previous().type==SEMICOLON) {
                return;
            }
            switch (peek().type) {
                case CLASS:
                case FUN:
                case VAR:
                case FOR:
                case IF:
                case WHILE:
                case PRINT:
                case RETURN:
                    return;
            }
            advance();
        }
    }
    bool match(std::vector<TOKEN_TYPE> operators) {
        for (auto op: operators) {
            if (checkTokenType(op)) {
                advance();
                return true;
            }
        }
        return false;
    }
    bool checkTokenType(TOKEN_TYPE tokenType) {
        if (isAtEnd()) return false;
        return peek().type==tokenType;
    }
    bool isAtEnd() {
        return peek().type==END;
    }
    Token peek() {
        return tokens[current];
    }
    Token previous() {
        return tokens[current-1];
    }
    Token advance() {
        if (!isAtEnd()) current++;
        return previous();
    }
};