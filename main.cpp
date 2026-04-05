#include "config.hpp"
#include "common.hpp"
#include "scanner.hpp"

int main(int argc, char** argv) {
    Scanner scanner(readFile("../test.txt"));
    scanner.scan();
    if (ScannerError::errorFound) {
        exit(1);
    }
    for (auto token: scanner.tokens) {
        std::cout<<token.toString()<<'\n';
    }
}