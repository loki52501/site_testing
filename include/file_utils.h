#ifndef FILE_UTILS_H
#define FILE_UTILS_H

#include <string>

// Read entire file contents into a string
std::string readFile(const std::string& filepath);

// Write string content to a file
void writeFile(const std::string& filepath, const std::string& content);

#endif // FILE_UTILS_H
