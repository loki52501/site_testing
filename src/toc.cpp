#include "toc.h"
#include <sstream>
#include <algorithm>
#include <cctype>

std::string generateTOC(const std::string& markdown) {
    std::stringstream ss(markdown);
    std::stringstream toc;
    std::string line;
    bool hasHeadings = false;

    toc << "<nav class=\"toc\">\n";
    toc << "    <h3>Table of Contents</h3>\n";
    toc << "    <ul>\n";

    while (std::getline(ss, line)) {
        if (!line.empty() && line[0] == '#') {
            // Count heading level
            size_t level = 0;
            while (level < line.length() && line[level] == '#') {
                level++;
            }

            // Skip the first h1 (title) - we don't want it in TOC
            if (level == 1 && !hasHeadings) {
                hasHeadings = true;
                continue;
            }

            // Extract heading text
            std::string headingText = line.substr(level);
            // Trim leading/trailing spaces
            size_t start = headingText.find_first_not_of(" \t");
            size_t end = headingText.find_last_not_of(" \t");
            if (start != std::string::npos && end != std::string::npos) {
                headingText = headingText.substr(start, end - start + 1);
            }

            // Create anchor from heading text (lowercase, replace spaces with hyphens)
            std::string anchor = headingText;
            std::transform(anchor.begin(), anchor.end(), anchor.begin(), ::tolower);
            std::replace(anchor.begin(), anchor.end(), ' ', '-');
            // Remove special characters
            anchor.erase(std::remove_if(anchor.begin(), anchor.end(),
                [](char c) { return !std::isalnum(c) && c != '-'; }), anchor.end());

            // Add indentation based on heading level
            std::string indent = "";
            for (size_t i = 2; i < level; i++) {
                indent += "    ";
            }

            toc << indent << "        <li class=\"toc-level-" << level << "\">"
                << "<a href=\"#" << anchor << "\">" << headingText << "</a></li>\n";
            hasHeadings = true;
        }
    }

    toc << "    </ul>\n";
    toc << "</nav>\n";

    return hasHeadings ? toc.str() : "";
}
