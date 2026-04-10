#pragma once
#include "common.hpp"
#include "config.hpp"
class Interpreter;
class Callable {
public:
    // we need to give the callable object a list of evaluated argument values
    // and the interpreter itself just in case
    virtual std::any call(Interpreter* interpreter, std::vector<std::any> arguments) = 0;
    virtual int arity() = 0; // number of arguments the function expects/supports
};

// native functions defined in the global scope:
class ClockFun: public Callable {
public:
    std::any call(Interpreter* interpreter, std::vector<std::any> arguments) override {
        // returns time in ms since epoch
        return (double)(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count());
    }
    int arity() override {
        return 0;
    }
};
class PrintFun: public Callable {
public:
    // technically speaking, the interpreter will throw a runtime error if we pass more than 1
    // argument, but allowing any number of arguments for print() function is a bit of a hassle
    std::any call(Interpreter* interpreter, std::vector<std::any> arguments) override {
        for (int i=0; i<arguments.size(); i++) {
            std::cout<<stringify(arguments[i]);
            if (i!=arguments.size()-1) {
                std::cout<<", ";
            }
        }
        std::cout<<'\n';
        return std::any{};
    }
    int arity() override {
        return 1;
    }
};