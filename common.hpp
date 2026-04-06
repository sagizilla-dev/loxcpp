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