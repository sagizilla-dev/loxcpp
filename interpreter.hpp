#pragma once
#include "config.hpp"
#include "common.hpp"
#include "token.hpp"
#include "ast.hpp"
#include "environment.hpp"
#include "callable.hpp"

class Interpreter; // forward declaration

// runtime representation of a function (wrapper around AST node FunDeclarationStmt)
class Function: public Callable {
public:
    Environment* closure; // this is the environment where the function was DECLARED
    FunDeclarationStmt* declaration;
    Function(FunDeclarationStmt* stmt, Environment* closure): declaration(stmt), closure(closure) {

    }
    std::any call(Interpreter* interpreter, std::vector<std::any> arguments) override;
    int arity() override {
        return declaration->parameters.size();
    }
    std::string toString() {
        return "<fn "+declaration->name.lexeme+">";
    }
};

class Interpreter: public ExprVisitor, public StmtVisitor {
public:
    Environment* global; // global scope is needed to define native functions
    Environment* env; // current environment, starts off as global
    // this stores variable resolutions, i.e. how many environment jumps we need to do from
    // the current environment to arrive at the correct variable declaration
    // this gets populated with unique expressions even if two expressions map to the same variable
    std::unordered_map<Expr*, int> locals;
    Interpreter() {
        global = new Environment;
        env = global;

        global->define("clock", (Callable*)(new ClockFun()));
        global->define("print", (Callable*)(new PrintFun()));
    }
    void interpret(std::vector<Stmt*> statements) {
        try {
            for (auto stmt: statements) {
                execute(stmt);
            }
        } catch (const RuntimeError& e) {
            std::cerr<<e.what()<<'\n';
        }
    }
    std::any visitBinaryExpr(BinaryExpr* expr) override {
        std::any left = evaluate(expr->left);
        std::any right = evaluate(expr->right);
        switch (expr->op.type) {
            case (MINUS): {
                checkNumberOperands(expr->op, left, right);
                return std::any_cast<double>(left)-std::any_cast<double>(right);
            }
            case (SLASH): {
                checkNumberOperands(expr->op, left, right);
                if (std::any_cast<double>(right)==0.0) {
                    throw RuntimeError(expr->op.line, "Division by zero");
                }
                return std::any_cast<double>(left)/std::any_cast<double>(right);
            }
            case (STAR): {
                checkNumberOperands(expr->op, left, right);
                return std::any_cast<double>(left)*std::any_cast<double>(right);
            }
            case (PLUS): {
                if (left.type()==typeid(double) && right.type()==typeid(double)) {
                    return std::any_cast<double>(left)+std::any_cast<double>(right);
                }
                if (left.type()==typeid(std::string) && right.type()==typeid(std::string)) {
                    return std::any_cast<std::string>(left)+std::any_cast<std::string>(right);
                }
                throw RuntimeError(expr->op.line, "Operands must be two numbers or strings");
            }
            case (GREATER): {
                checkNumberOperands(expr->op, left, right);
                return std::any_cast<double>(left)>std::any_cast<double>(right);
            }
            case (GREATER_EQUAL): {
                checkNumberOperands(expr->op, left, right);
                return std::any_cast<double>(left)>=std::any_cast<double>(right);
            }
            case (LESS): {
                checkNumberOperands(expr->op, left, right);
                return std::any_cast<double>(left)<std::any_cast<double>(right);
            }
            case (LESS_EQUAL): {
                checkNumberOperands(expr->op, left, right);
                return std::any_cast<double>(left)<=std::any_cast<double>(right);
            }
            case (BANG_EQUAL): {
                checkNumberOperands(expr->op, left, right);
                return !isEqual(left, right);
            }
            case (EQUAL_EQUAL): {
                checkNumberOperands(expr->op, left, right);
                return isEqual(left, right);
            }
        }
        return std::any{}; // unreachable
    }
    bool isEqual(std::any left, std::any right) {
        // two nils are equal
        if (!left.has_value() && !right.has_value()) {
            return true;
        }
        if (!left.has_value() || !right.has_value()) {
            return false;
        }
        if (left.type()==typeid(double) && right.type()==typeid(double)) {
            return std::any_cast<double>(left)==std::any_cast<double>(right);
        }
        if (left.type()==typeid(std::string) && right.type()==typeid(std::string)) {
            return std::any_cast<std::string>(left)==std::any_cast<std::string>(right);
        }
        return false; // unreachable
    }
    std::any visitGroupingExpr(GroupingExpr* expr) override {
        return evaluate(expr->expr);
    };
    std::any visitUnaryExpr(UnaryExpr* expr) override {
        std::any value = evaluate(expr->expr);
        switch (expr->op.type) {
            case (MINUS): {
                checkNumberOperand(expr->op, value);
                return -std::any_cast<double>(value);
            }
            case (BANG): {
                return !isTruthy(value);
            }
        }
        return std::any{}; // unreachable
    }
    std::any visitExpressionStmt(ExpressionStmt* stmt) override {
        evaluate(stmt->expr);
        return std::any{};
    }
    std::any visitPrintStmt(PrintStmt* stmt) override {
        std::any value = evaluate(stmt->expr);
        std::cout<<stringify(value)<<'\n';
        return std::any{};
    }
    void checkNumberOperands(Token op, std::any value1, std::any value2) {
        if (value1.type()!=typeid(double) || value2.type()!=typeid(double)) {
            throw RuntimeError(op.line, "Operands must be numbers");
        }
    }
    void checkNumberOperand(Token op, std::any value) {
        if (value.type()!=typeid(double)) {
            throw RuntimeError(op.line, "Operand must be a number");
        }
    }
    bool isTruthy(std::any value) {
        // only false and nil are false, the rest is true
        if (!value.has_value()) {
            return false;
        }
        if (value.type()==typeid(bool)) {
            return std::any_cast<bool>(value);
        }
        return true;
    }
    std::any visitLiteralExpr(LiteralExpr* expr) override {
        return expr->value;
    }
    std::any visitVarDeclarationStmt(VarDeclarationStmt* stmt) override {
        std::any value;
        if (stmt->initializer) {
            value = evaluate(stmt->initializer);
        }
        // if a variable has never been initiaized, we assign nil to it
        env->define(stmt->name.lexeme, value);
        return nullptr;
    }
    std::any visitVariableExpr(VariableExpr* expr) override {
        if (locals.count(expr)) {
            int depth = locals[expr];
            return env->getAt(depth, expr->name.lexeme);
        } else {
            return global->get(expr->name);
        }
    }
    std::any visitIfStmt(IfStmt* stmt) override {
        if (isTruthy(evaluate(stmt->condition))) {
            execute(stmt->thenStatement);
        } else if (stmt->elseStatement) {
            execute(stmt->elseStatement);
        }
        return std::any{};
    }
    std::any visitWhileStmt(WhileStmt* stmt) override {
        while (isTruthy(evaluate(stmt->condition))) {
            execute(stmt->body);
        }
        return std::any{};
    }
    std::any visitLogicExpr(LogicExpr* expr) override {
        std::any left = evaluate(expr->left);
        // short-circuit evaluation
        if (expr->op.type==OR) {
            if (isTruthy(left)) {
                // we return the result of the expression itself, so that
                // ("hello" or nil) returns "hello" instead of pure true
                return left;
            }
        } else {
            if (!isTruthy(left)) {
                return left;
            }
        }
        return evaluate(expr->right);
    }
    void resolve(Expr* expr, int depth) {
        locals[expr]=depth;
    }
    std::any visitAssignExpr(AssignExpr* expr) override {
        std::any value = evaluate(expr->value);
        if (locals.count(expr)) {
            int depth = locals[expr];
            env->assignAt(depth, expr->name, value);
        } else {
            global->assign(expr->name, value);
        }
        return value;
    }
    std::any visitBlockStmt(BlockStmt* stmt) override {
        executeBlock(stmt->statements, new Environment(env));
        return std::any{};
    }
    std::any visitFunDeclarationStmt(FunDeclarationStmt* stmt) override {
        // we basically convert compile-time representation (stmt) into runtime (Function)
        Callable* function = new Function(stmt, env);
        env->define(stmt->name.lexeme, function); // current environment now stores the reference to the function
        // now, if we retrieve the function object, we can use it even if we are out of the scope where it was declared.
        // note that because of closures the function will alter values within the original environment, as the function itself
        // also stores its closure
        return std::any{};
    }
    std::any visitCallExpr(CallExpr* expr) override {
        std::any callee = evaluate(expr->callee); // used to evaluate things like getCallee()()()...
        std::vector<std::any> arguments;
        for (auto arg: expr->arguments) {
            arguments.push_back(evaluate(arg));
        }
        if (callee.type()!=typeid(Callable*)) {
            throw RuntimeError(expr->paren.line, "Can only call functions and classes");
        }
        Callable* fun = std::any_cast<Callable*>(callee);
        if (arguments.size() != fun->arity()) {
            throw RuntimeError(expr->paren.line, "Expected " + std::to_string(fun->arity()) + 
                " arguments but received " + std::to_string(arguments.size()));
        }
        return fun->call(this, arguments);
    }
    std::any visitReturnStmt(ReturnStmt* stmt) override {
        std::any value;
        if (stmt->value != nullptr) {
            value = evaluate(stmt->value);
        }
        throw Return(value);
    }
    void executeBlock(std::vector<Stmt*> statements, Environment* environment) {
        // save the previous environment to restore it once we exit the block
        Environment* previous = this->env;
        try {
            this->env = environment;
            for (auto stmt: statements) {
                execute(stmt);
            }
        } catch (const Return& r) {
            // when we encounter a return value, it means the callable object has reached
            // its scope's end, so we need to restore the original "pre-call" environment
            // and do stack unwinding to get back to the original function caller
            // so we restore the environment back to the environment where the function was called
            this->env = previous;
            throw; // rethrow the return
        } catch (const RuntimeError& e) {
            throw e;
        }
        this->env = previous;
    }
    std::any evaluate(Expr* expr) {
        return expr->accept(this);
    }
    void execute(Stmt* stmt) {
        stmt->accept(this);
    }
};

std::any Function::call(Interpreter* interpreter, std::vector<std::any> arguments) {
    // this is the environment where parameters live
    // every function call gets its own environment, otherwise recursion would break.
    // this new environment "inherits" from function's closure, and stores parameters and everything
    // declared within the body of the function
    Environment* env = new Environment(closure);
    for (int i=0; i<declaration->parameters.size(); i++) {
        env->define(declaration->parameters[i].lexeme, arguments[i]);
    }
    try {
        // execute the statements, but note that the body is not BlockStmt
        interpreter->executeBlock(declaration->body, env);
    } catch (const Return& r) {
        return r.value;
    }
    // function with no return value or return statement returns nil
    return std::any{};
}