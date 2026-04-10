#pragma once
#include "config.hpp"
#include "common.hpp"

enum TOKEN_TYPE {
    LEFT_PAREN, RIGHT_PAREN, LEFT_BRACE, RIGHT_BRACE,
    COMMA, DOT, MINUS, PLUS, SEMICOLON, SLASH, STAR,

    BANG, BANG_EQUAL,
    EQUAL, EQUAL_EQUAL,
    GREATER, GREATER_EQUAL,
    LESS, LESS_EQUAL,

    IDENTIFIER, STRING, NUMBER,

    AND, CLASS, ELSE, FALSE, FUN, FOR, IF, NIL, OR,
    RETURN, SUPER, THIS, TRUE, VAR, WHILE,

    END
};
struct Token {
    TOKEN_TYPE type;
    std::string lexeme;
    std::any literal; // value is defined for boolean, strings and numbers, and not defined for nil expression
    int line;
    Token(TOKEN_TYPE type, std::string lexeme, std::any literal, int line):
    type(type), lexeme(lexeme), literal(literal), line(line) {
        
    }
    std::string toString() {
        std::string str = "(type " + std::to_string(type) + ", line " + std::to_string(line) + ") -> " + lexeme;
        if (literal.has_value()) {
            if (literal.type()==typeid(double)) {
                str+=" ("+std::to_string(std::any_cast<double>(literal)) + ")";
            } else {
                str+=" ("+std::any_cast<std::string>(literal) + ")";
            }
        } else if (type==NIL) {
            str+=" (nill)";
        }
        return str;
    }
};