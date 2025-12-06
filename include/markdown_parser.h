#ifndef MARKDOWN_PARSER_H
#define MARKDOWN_PARSER_H

#include <string>
#include <vector>
#include <map>

class MarkdownParser {
public:
    MarkdownParser();

    // Main conversion function
    std::string convertToHTML(const std::string& markdown);

private:
    // Parse individual markdown elements
    std::string parseHeaders(const std::string& line);
    std::string parseBold(const std::string& line);
    std::string parseItalic(const std::string& line);
    std::string parseLinks(const std::string& line);
    std::string parseImages(const std::string& line);
    std::string parseInlineCode(const std::string& line);
    std::string parseList(const std::string& line, bool& inList, bool& isOrdered);
    std::string parseCodeBlock(const std::string& line, bool& inCodeBlock, std::string& codeContent);
    std::string parseParagraph(const std::string& line);

    // Helper functions
    bool isHeader(const std::string& line);
    bool isListItem(const std::string& line);
    bool isCodeBlockDelimiter(const std::string& line);
    bool isEmptyLine(const std::string& line);
    std::string trim(const std::string& str);
    std::string escapeHTML(const std::string& str);
};

#endif
