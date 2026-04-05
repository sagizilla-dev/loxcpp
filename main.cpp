#include "config.hpp"
#include "common.hpp"
#include "scanner.hpp"
#include "astPrinter.hpp"

int main(int argc, char** argv) {
    Scanner scanner(readFile("../test.txt"));
    scanner.scan();
    if (ScannerError::errorFound) {
        exit(1);
    }
    ASTPrinter astPrinter;
    astPrinter.print(new BinaryExpr(
        new UnaryExpr(new LiteralExpr(123.0), new Token(NUMBER, "-", std::any{}, 1)),
        new GroupingExpr(new LiteralExpr(45.67)),
        new Token(STAR, "*", std::any{}, 1)
    ));
}