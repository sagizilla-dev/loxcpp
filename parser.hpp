#pragma once
#include "config.hpp"
#include "token.hpp"
#include "ast.hpp"

struct Parser {
    std::vector<Token> tokens;
    int current = 0;
    std::vector<Stmt*> statements;
    Parser(std::vector<Token> tokens): tokens(tokens) {

    }
    std::vector<Stmt*> parse() {
        while (!isAtEnd()) {
            statements.push_back(declaration());
        }
        return statements;
    }
    Stmt* declaration() {
        try {
            if (match({VAR})) {
                return varDeclaration();
            }
            return statement();
        } catch (const SyntaxError& e) {
            std::cerr<<e.what()<<'\n';
            // panic mode, we need to synchronize on a statement boundary
            synchronize();
            return nullptr;
        }
    }
    Stmt* varDeclaration() {
        Token name = consume(IDENTIFIER, "Expected variable name");
        Expr* initializer = nullptr;
        if (match({EQUAL})) {
            initializer = expression();
        }
        consume(SEMICOLON, "Expected ; after statement");
        return new VarDeclarationStmt(name, initializer);
    }
    Stmt* statement() {
        if (match({PRINT})) {
            return printStatement();
        }
        if (match({LEFT_BRACE})) {
            return new BlockStmt(blockStatement());
        }
        return expressionStatement();
    }
    std::vector<Stmt*> blockStatement() {
        std::vector<Stmt*> statements;
        while (!isAtEnd() && !checkTokenType(RIGHT_BRACE)) {
            statements.push_back(declaration());
        }
        consume(RIGHT_BRACE, "Expected } after block");
        return statements;
    }
    Stmt* printStatement() {
        Expr* value = expression();
        consume(SEMICOLON, "Expected ; after statement");
        return new PrintStmt(value);
    }
    Stmt* expressionStatement() {
        Expr* value = expression();
        consume(SEMICOLON, "Expected ; after statement");
        return new ExpressionStmt(value);
    }
    Expr* expression() {
        return assignment();
    }
    Expr* assignment() {
        // assignment expression is tricky since the left-hand side might be any expression, and we don't
        // know we are parsing an assignment expression until we stubmle upon =, which may occur however many tokens ahead
        // i.e node.next.prev.prev.prev.value = 2;
        // this is important since we cannot accept a program that does "a+b=2;"
        // we must verify the left-hand side is an expression that is an l-value, not r-value
        Expr* expr = equality();

        if (match({EQUAL})) {
            Token equals = previous();
            Expr* value = assignment(); // this recursive call makes the whole operator right-associative
            // dynamic cast checks if the expression was actually a variable expression
            // if not, it is an error!
            if (VariableExpr* var = dynamic_cast<VariableExpr*>(expr)) {
                Token name = var->name;
                return new AssignExpr(name, value);
            }
            throw SyntaxError(equals.line, "Invalid assignment target");
        }
        // every valid assignment target is also valid syntax as a normal expression
        return expr;
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
        if (match({IDENTIFIER})) {
            return new VariableExpr(previous());
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