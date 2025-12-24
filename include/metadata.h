#ifndef METADATA_H
#define METADATA_H

#include <string>
#include <ctime>

// Extract title from markdown content (first # heading)
std::string extractTitle(const std::string& markdown);

// Extract excerpt from markdown content (first paragraph after title)
std::string extractExcerpt(const std::string& markdown, size_t maxLength = 200);

// Get formatted file modification date
std::string getFileModificationDate(const std::string& filepath);

// Get file modification timestamp
std::time_t getFileModificationTimestamp(const std::string& filepath);

#endif // METADATA_H
