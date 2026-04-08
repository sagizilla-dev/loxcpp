#pragma once
#include "config.hpp"
#include "common.hpp"
#include "token.hpp"

struct Environment {
    Environment* enclosing; // outer environment
    std::unordered_map<std::string, std::any> values;
    Environment() {
        enclosing = nullptr;
    }
    Environment(Environment* enclosing):enclosing(enclosing) {

    }
    void define(std::string name, std::any value) {
        values[name]=value; // this allows us to redefine a variable via declaration
    }
    std::any get(Token token) {
        if (values.count(token.lexeme)) {
            return values[token.lexeme];
        }
        if (enclosing) {
            // this allows us to check outter scopes until we either find the variable or throw an error
            return enclosing->get(token);
        }
        // so referring to a non-declared variable should be a syntax error, but in this case it is
        // a runtime error since, if we define it to be a static error,
        // it becomes a lot harder to define recursive functions.
        // note that using a variable is not the same as referring to it, i.e
        // referring to a variable inside a function should be allowed.
        // single-function recursion should be allowed, and even functions that call each other are
        // allowed.
        // so now it is allowed to refer to an undefined variable as long as it is not evaluated,
        // but in such a case we will throw a runtime error
        throw RuntimeError(token.line, "Undefined variable: " + token.lexeme);
    }
    void assign(Token token, std::any value) {
        if (values.count(token.lexeme)) {
            values[token.lexeme]=value;
            return;
        }
        if (enclosing) {
            enclosing->assign(token, value);
            return;
        }
        // assign operator is not allowed to define a variable!
        throw RuntimeError(token.line, "Undefined variable: " + token.lexeme);
    }
};