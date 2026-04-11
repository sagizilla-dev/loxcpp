#include "config.hpp"
#include "common.hpp"
#include "scanner.hpp"
// #include "astPrinter.hpp"
#include "parser.hpp"
#include "interpreter.hpp"
#include "resolver.hpp"

int main(int argc, char** argv) {
    Scanner scanner(readFile("../test.txt"));
    scanner.scan();
    if (ScannerError::errorFound) {
        exit(1);
    }
    Parser parser(scanner.tokens);
    std::vector<Stmt*> statements = parser.parse();
    if (SyntaxError::errorFound) {
        exit(1);
    }
    // ASTPrinter astPrinter;
    // astPrinter.print(expr);
    Interpreter interpreter;
    Resolver resolver(&interpreter);
    resolver.resolve(statements);
    interpreter.interpret(statements);
    if (RuntimeError::errorFound) {
        exit(1);
    }
}