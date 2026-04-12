#pragma once
#include "common.hpp"
#include "config.hpp"
#include "callable.hpp"
class Interpreter;
class ClassInstance;
class Class: public Callable {
public:
    std::string name;
    std::unordered_map<std::string, Function*> methods;
    Class(std::string name, std::unordered_map<std::string, Function*> methods): name(name), methods(methods)  {

    }
    std::string toString() {
        return name;
    }
    std::any call(Interpreter* interpreter, std::vector<std::any> arguments);
    int arity() {
        Function* initializer = findMethod("init");
        if (initializer) {
            return initializer->arity();
        }
        return 0;
    }
    Function* findMethod(std::string name) {
        if (methods.count(name)) {
            return methods[name];
        }
        return nullptr;
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
        Callable* method = klass->findMethod(name.lexeme);
        if (method) {
            return ((Function*)method)->bind(this);
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
    Function* initializer = findMethod("init");
    if (initializer) {
        // from the very beginning we bind the initializer to class instance
        // and then call the initializer itself
        // since "this" points to the instance, initializer is free
        // to change it however it needs
        (initializer->bind(instance))->call(interpreter, arguments);
    }
    return instance;
}