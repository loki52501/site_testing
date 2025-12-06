#include "../include/markdown_parser.h"
#include <regex>
#include <sstream>
#include <algorithm>

MarkdownParser::MarkdownParser() {}

std::string MarkdownParser::convertToHTML(const std::string& markdown) {
    std::stringstream html;
    std::stringstream ss(markdown);
    std::string line;

    bool inCodeBlock = false;
    bool inList = false;
    bool isOrdered = false;
    bool inParagraph = false;
    bool inTable = false;
    std::string codeContent;
    std::vector<std::string> tableLines;

    while (std::getline(ss, line)) {
        // Handle code blocks
        if (isCodeBlockDelimiter(line)) {
            std::string codeHtml = parseCodeBlock(line, inCodeBlock, codeContent);
            if (!codeHtml.empty()) {
                if (inList) {
                    html << (isOrdered ? "</ol>\n" : "</ul>\n");
                    inList = false;
                }
                if (inParagraph) {
                    html << "</p>\n";
                    inParagraph = false;
                }
                if (inTable) {
                    html << parseTable(tableLines);
                    tableLines.clear();
                    inTable = false;
                }
                html << codeHtml;
            }
            continue;
        }

        if (inCodeBlock) {
            codeContent += line + "\n";
            continue;
        }

        // Handle tables
        if (isTableRow(line)) {
            if (inList) {
                html << (isOrdered ? "</ol>\n" : "</ul>\n");
                inList = false;
            }
            if (inParagraph) {
                html << "</p>\n";
                inParagraph = false;
            }
            inTable = true;
            tableLines.push_back(line);
            continue;
        } else if (inTable) {
            // End of table
            html << parseTable(tableLines);
            tableLines.clear();
            inTable = false;
        }

        // Handle empty lines
        if (isEmptyLine(line)) {
            if (inList) {
                html << (isOrdered ? "</ol>\n" : "</ul>\n");
                inList = false;
            }
            if (inParagraph) {
                html << "</p>\n";
                inParagraph = false;
            }
            continue;
        }

        // Handle headers
        if (isHeader(line)) {
            if (inList) {
                html << (isOrdered ? "</ol>\n" : "</ul>\n");
                inList = false;
            }
            if (inParagraph) {
                html << "</p>\n";
                inParagraph = false;
            }
            html << parseHeaders(line) << "\n";
            continue;
        }

        // Handle lists
        if (isListItem(line)) {
            std::string listHtml = parseList(line, inList, isOrdered);
            if (inParagraph) {
                html << "</p>\n";
                inParagraph = false;
            }
            html << listHtml << "\n";
            continue;
        }

        // Handle regular paragraphs
        if (inList) {
            html << (isOrdered ? "</ol>\n" : "</ul>\n");
            inList = false;
        }

        if (!inParagraph) {
            html << "<p>";
            inParagraph = true;
        } else {
            html << " ";
        }

        std::string processedLine = line;
        processedLine = parseImages(processedLine);
        processedLine = parseLinks(processedLine);
        processedLine = parseBold(processedLine);
        processedLine = parseItalic(processedLine);
        processedLine = parseInlineCode(processedLine);

        html << processedLine;
    }

    // Close any open tags
    if (inList) {
        html << (isOrdered ? "</ol>\n" : "</ul>\n");
    }
    if (inParagraph) {
        html << "</p>\n";
    }
    if (inTable) {
        html << parseTable(tableLines);
    }
    if (inCodeBlock && !codeContent.empty()) {
        html << "<pre><code>" << escapeHTML(codeContent) << "</code></pre>\n";
    }

    return html.str();
}

std::string MarkdownParser::parseHeaders(const std::string& line) {
    size_t level = 0;
    size_t i = 0;

    while (i < line.length() && line[i] == '#') {
        level++;
        i++;
    }

    if (level > 0 && level <= 6 && i < line.length() && line[i] == ' ') {
        std::string content = trim(line.substr(i + 1));
        content = parseImages(content);
        content = parseBold(content);
        content = parseItalic(content);
        content = parseLinks(content);
        content = parseInlineCode(content);

        return "<h" + std::to_string(level) + ">" + content + "</h" + std::to_string(level) + ">";
    }

    return line;
}

std::string MarkdownParser::parseBold(const std::string& line) {
    std::regex boldRegex("\\*\\*([^*]+)\\*\\*");
    return std::regex_replace(line, boldRegex, "<strong>$1</strong>");
}

std::string MarkdownParser::parseItalic(const std::string& line) {
    std::regex italicRegex("\\*([^*]+)\\*");
    return std::regex_replace(line, italicRegex, "<em>$1</em>");
}

std::string MarkdownParser::parseLinks(const std::string& line) {
    std::regex linkRegex("\\[([^\\]]+)\\]\\(([^)]+)\\)");
    return std::regex_replace(line, linkRegex, "<a href=\"$2\">$1</a>");
}

std::string MarkdownParser::parseImages(const std::string& line) {
    std::regex imageRegex("!\\[([^\\]]*)\\]\\(([^)]+)\\)");
    return std::regex_replace(line, imageRegex, "<img src=\"$2\" alt=\"$1\">");
}

std::string MarkdownParser::parseInlineCode(const std::string& line) {
    std::regex codeRegex("`([^`]+)`");
    return std::regex_replace(line, codeRegex, "<code>$1</code>");
}

std::string MarkdownParser::parseList(const std::string& line, bool& inList, bool& isOrdered) {
    std::string trimmedLine = trim(line);
    std::stringstream html;

    bool currentIsOrdered = (trimmedLine.length() > 0 && std::isdigit(trimmedLine[0]));

    if (!inList) {
        html << (currentIsOrdered ? "<ol>\n" : "<ul>\n");
        inList = true;
        isOrdered = currentIsOrdered;
    } else if (isOrdered != currentIsOrdered) {
        html << (isOrdered ? "</ol>\n" : "</ul>\n");
        html << (currentIsOrdered ? "<ol>\n" : "<ul>\n");
        isOrdered = currentIsOrdered;
    }

    size_t contentStart = 0;
    if (currentIsOrdered) {
        contentStart = trimmedLine.find('.') + 1;
    } else {
        contentStart = (trimmedLine[0] == '-' || trimmedLine[0] == '*') ? 1 : 0;
    }

    std::string content = trim(trimmedLine.substr(contentStart));
    content = parseImages(content);
    content = parseBold(content);
    content = parseItalic(content);
    content = parseLinks(content);
    content = parseInlineCode(content);

    html << "  <li>" << content << "</li>";

    return html.str();
}

std::string MarkdownParser::parseCodeBlock(const std::string& line, bool& inCodeBlock, std::string& codeContent) {
    if (isCodeBlockDelimiter(line)) {
        if (inCodeBlock) {
            std::string html = "<pre><code>" + escapeHTML(codeContent) + "</code></pre>";
            codeContent.clear();
            inCodeBlock = false;
            return html;
        } else {
            inCodeBlock = true;
            return "";
        }
    }
    return "";
}

std::string MarkdownParser::parseParagraph(const std::string& line) {
    std::string content = line;
    content = parseImages(content);
    content = parseBold(content);
    content = parseItalic(content);
    content = parseLinks(content);
    content = parseInlineCode(content);

    return "<p>" + content + "</p>";
}

bool MarkdownParser::isHeader(const std::string& line) {
    if (line.empty()) return false;
    if (line[0] != '#') return false;

    size_t i = 0;
    while (i < line.length() && line[i] == '#') {
        i++;
    }

    return i > 0 && i <= 6 && i < line.length() && line[i] == ' ';
}

bool MarkdownParser::isListItem(const std::string& line) {
    std::string trimmedLine = trim(line);
    if (trimmedLine.empty()) return false;

    if (trimmedLine[0] == '-' || trimmedLine[0] == '*') {
        return trimmedLine.length() > 1 && trimmedLine[1] == ' ';
    }

    if (std::isdigit(trimmedLine[0])) {
        size_t dotPos = trimmedLine.find('.');
        return dotPos != std::string::npos && dotPos < trimmedLine.length() - 1;
    }

    return false;
}

bool MarkdownParser::isCodeBlockDelimiter(const std::string& line) {
    std::string trimmedLine = trim(line);
    return trimmedLine.find("```") == 0;
}

bool MarkdownParser::isEmptyLine(const std::string& line) {
    return trim(line).empty();
}

std::string MarkdownParser::trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";

    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

std::string MarkdownParser::escapeHTML(const std::string& str) {
    std::string result;
    for (char c : str) {
        switch (c) {
            case '<': result += "&lt;"; break;
            case '>': result += "&gt;"; break;
            case '&': result += "&amp;"; break;
            case '"': result += "&quot;"; break;
            case '\'': result += "&#39;"; break;
            default: result += c;
        }
    }
    return result;
}

bool MarkdownParser::isTableRow(const std::string& line) {
    std::string trimmedLine = trim(line);
    if (trimmedLine.empty()) return false;

    // A table row starts and ends with |
    if (trimmedLine[0] == '|' && trimmedLine.find('|', 1) != std::string::npos) {
        return true;
    }

    return false;
}

bool MarkdownParser::isTableDelimiter(const std::string& line) {
    std::string trimmedLine = trim(line);
    if (!isTableRow(trimmedLine)) return false;

    // Check if the line contains only |, -, and spaces
    for (char c : trimmedLine) {
        if (c != '|' && c != '-' && c != ' ' && c != ':') {
            return false;
        }
    }

    return true;
}

std::vector<std::string> MarkdownParser::splitTableRow(const std::string& line) {
    std::vector<std::string> cells;
    std::string trimmedLine = trim(line);

    // Remove leading and trailing |
    if (!trimmedLine.empty() && trimmedLine[0] == '|') {
        trimmedLine = trimmedLine.substr(1);
    }
    if (!trimmedLine.empty() && trimmedLine.back() == '|') {
        trimmedLine = trimmedLine.substr(0, trimmedLine.length() - 1);
    }

    // Split by |
    std::stringstream ss(trimmedLine);
    std::string cell;

    while (std::getline(ss, cell, '|')) {
        cells.push_back(trim(cell));
    }

    return cells;
}

std::string MarkdownParser::parseTable(std::vector<std::string>& tableLines) {
    if (tableLines.empty()) return "";

    std::stringstream html;
    html << "<table>\n";

    bool hasHeader = false;

    // Check if the second line is a delimiter (indicating header row)
    if (tableLines.size() > 1 && isTableDelimiter(tableLines[1])) {
        hasHeader = true;
    }

    // Parse header if present
    if (hasHeader) {
        html << "  <thead>\n    <tr>\n";
        std::vector<std::string> cells = splitTableRow(tableLines[0]);
        for (const auto& cell : cells) {
            std::string processedCell = cell;
            processedCell = parseImages(processedCell);
            processedCell = parseBold(processedCell);
            processedCell = parseItalic(processedCell);
            processedCell = parseLinks(processedCell);
            processedCell = parseInlineCode(processedCell);
            html << "      <th>" << processedCell << "</th>\n";
        }
        html << "    </tr>\n  </thead>\n";
    }

    // Parse body rows
    html << "  <tbody>\n";
    for (size_t i = (hasHeader ? 2 : 0); i < tableLines.size(); i++) {
        if (isTableDelimiter(tableLines[i])) continue;

        html << "    <tr>\n";
        std::vector<std::string> cells = splitTableRow(tableLines[i]);
        for (const auto& cell : cells) {
            std::string processedCell = cell;
            processedCell = parseImages(processedCell);
            processedCell = parseBold(processedCell);
            processedCell = parseItalic(processedCell);
            processedCell = parseLinks(processedCell);
            processedCell = parseInlineCode(processedCell);
            html << "      <td>" << processedCell << "</td>\n";
        }
        html << "    </tr>\n";
    }
    html << "  </tbody>\n";

    html << "</table>\n";
    return html.str();
}
