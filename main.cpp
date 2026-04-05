#include <iostream>
#include <fstream>
#include <sstream>

std::string readFile(std::string filename) {
    std::ifstream file(filename);
    std::stringstream stream;
    stream << file.rdbuf();
    return stream.str();
}

int main(int argc, char** argv) {
    std::cout<<readFile("../test.txt");
}