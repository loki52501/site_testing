#include "file_utils.h"
#include <iostream>
#include <fstream>
#include <sstream>

std::string readFile(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filepath << std::endl;
        return "";
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

void writeFile(const std::string& filepath, const std::string& content) {
    std::ofstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Error: Could not write to file " << filepath << std::endl;
        return;
    }

    file << content;
    file.close();
    std::cout << "Generated: " << filepath << std::endl;
}
