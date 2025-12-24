#include "metadata.h"
#include <sstream>
#include <filesystem>
#include <chrono>
#include <iomanip>

namespace fs = std::filesystem;

std::string extractTitle(const std::string& markdown) {
    std::stringstream ss(markdown);
    std::string line;

    while (std::getline(ss, line)) {
        if (line.length() > 2 && line[0] == '#' && line[1] == ' ') {
            return line.substr(2);
        }
    }

    return "Untitled";
}

std::string extractExcerpt(const std::string& markdown, size_t maxLength) {
    std::stringstream ss(markdown);
    std::string line;
    std::string excerpt;
    bool skippedHeader = false;
    bool skippedSecondLine = false;

    while (std::getline(ss, line)) {
        // Skip lines starting with # (headers)
        if (!line.empty() && line[0] == '#') {
            skippedHeader = true;
            continue;
        }

        // Skip empty lines before we find content
        if (line.empty()) {
            continue;
        }

        // Skip the second line (first non-empty line after header)
        if (skippedHeader && !skippedSecondLine) {
            skippedSecondLine = true;
            continue;
        }

        // Now accumulate paragraph text
        if (!excerpt.empty()) {
            excerpt += " ";
        }
        excerpt += line;

        // Stop if we have enough text
        if (excerpt.length() >= maxLength) {
            break;
        }
    }

    // Truncate to maxLength and add ellipsis if needed
    if (excerpt.length() > maxLength) {
        excerpt = excerpt.substr(0, maxLength);
        size_t lastSpace = excerpt.find_last_of(" ");
        if (lastSpace != std::string::npos) {
            excerpt = excerpt.substr(0, lastSpace);
        }
        excerpt += "...";
    }

    return excerpt;
}

std::string getFileModificationDate(const std::string& filepath) {
    auto ftime = fs::last_write_time(filepath);
    auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
        ftime - fs::file_time_type::clock::now() + std::chrono::system_clock::now()
    );
    std::time_t cftime = std::chrono::system_clock::to_time_t(sctp);

    std::stringstream ss;
    ss << std::put_time(std::localtime(&cftime), "%B %d, %Y at %I:%M %p");
    return ss.str();
}

std::time_t getFileModificationTimestamp(const std::string& filepath) {
    auto ftime = fs::last_write_time(filepath);
    auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
        ftime - fs::file_time_type::clock::now() + std::chrono::system_clock::now()
    );
    return std::chrono::system_clock::to_time_t(sctp);
}
