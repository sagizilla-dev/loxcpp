#pragma once
#include "config.hpp"

inline std::string readFile(std::string filename) {
    std::ifstream file(filename);
    std::stringstream str;
    str << file.rdbuf();
    return str.str();
}

class ScannerError: public std::exception {
public:
    int line;
    std::string message;
    std::string errorMessage;
    static bool errorFound;
    ScannerError(int line, std::string errorMessage): line(line), errorMessage(errorMessage) {
        errorFound = true;
        message = "Scanner error[" + std::to_string(line)+"] -> " + errorMessage;
    }
    const char* what() const noexcept override {
        return message.c_str();
    }
};
bool ScannerError::errorFound = false;

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

class ResolverError: public std::exception {
public:
    int line;
    std::string message;
    std::string errorMessage;
    static bool errorFound;
    ResolverError(int line, std::string errorMessage): line(line), errorMessage(errorMessage) {
        errorFound = true;
        message = "ResolverError error[" + std::to_string(line)+"] -> " + errorMessage;
    }
    const char* what() const noexcept override {
        return message.c_str();
    }
};
bool ResolverError::errorFound = false;

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

// this return class is only needed to get out of a function
// no matter how deep we are
// stack unwinding lets us get back and "catch" the return right where the function is called
class Return: public std::exception {
public:
    std::any value;
    Return(std::any value):value(value) {
        
    }
};
inline std::string stringify(std::any value) {
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