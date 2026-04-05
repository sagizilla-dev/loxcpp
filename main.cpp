#include "config.hpp"
#include "common.hpp"
#include "scanner.hpp"
#include "astPrinter.hpp"
#include "parser.hpp"
#include "interpreter.hpp"

int main(int argc, char** argv) {
    Scanner scanner(readFile("../test.txt"));
    scanner.scan();
    if (ScannerError::errorFound) {
        exit(1);
    }
    Parser parser(scanner.tokens);
    Expr* expr = parser.parse();
    if (SyntaxError::errorFound) {
        exit(1);
    }
    ASTPrinter astPrinter;
    astPrinter.print(expr);
    Interpreter interpreter;
    interpreter.interpret(expr);
    if (RuntimeError::errorFound) {
        exit(1);
    }
}