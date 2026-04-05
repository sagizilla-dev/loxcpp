#pragma once
#include "config.hpp"

inline std::string readFile(std::string filename) {
    std::ifstream file(filename);
    std::stringstream str;
    str << file.rdbuf();
    return str.str();
}