#pragma once
#include "common.hpp"
#include "config.hpp"
#include "callable.hpp"
class Interpreter;
class Class: public Callable {
public:
    std::string name;
    Class(std::string name): name(name) {

    }
    std::string toString() {
        return name;
    }
    std::any call(Interpreter* interpreter, std::vector<std::any> arguments) {
        return std::any{};
    }
    int arity() {
        return 0;
    }
};