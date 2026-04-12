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
    std::any call(Interpreter* interpreter, std::vector<std::any> arguments);
    int arity() {
        return 0;
    }
};
class ClassInstance {
public:
    Class* klass;
    std::unordered_map<std::string, std::any> fields;
    ClassInstance(Class* klass):klass(klass) {

    }
    std::string toString() {
        return klass->name+" instance";
    }
    std::any get(Token name) {
        if (fields.count(name.lexeme)) {
            return fields[name.lexeme];
        }
        throw RuntimeError(name.line, "Undefined property '"+name.lexeme+"'");
    }
    std::any set(Token name, std::any value) {
        // we allow free dynamic field creation, so no need to see if the key is present
        fields[name.lexeme]=value;
        return std::any{};
    }
};
std::any Class::call(Interpreter* interpreter, std::vector<std::any> arguments) {
    ClassInstance* instance = new ClassInstance(this);
    return instance;
}