#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <vector>
#include <map>
#include "../include/markdown_parser.h"

namespace fs = std::filesystem;

struct Page {
    std::string filename;
    std::string title;
    std::string content;
    std::string outputPath;
};

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

std::string applyTemplate(const std::string& templateContent, const std::string& title,
                          const std::string& content, const std::vector<Page>& pages) {
    std::string result = templateContent;

    // Replace title placeholder
    size_t pos = result.find("{{TITLE}}");
    while (pos != std::string::npos) {
        result.replace(pos, 9, title);
        pos = result.find("{{TITLE}}", pos + title.length());
    }

    // Replace content placeholder
    pos = result.find("{{CONTENT}}");
    if (pos != std::string::npos) {
        result.replace(pos, 11, content);
    }

    // Generate navigation
    std::stringstream nav;
    for (const auto& page : pages) {
        nav << "<a href=\"" << page.outputPath << "\">" << page.title << "</a>";
    }

    pos = result.find("{{NAV}}");
    if (pos != std::string::npos) {
        result.replace(pos, 7, nav.str());
    }

    return result;
}

int main(int argc, char* argv[]) {
    std::cout << "=== Markdown Static Site Generator ===" << std::endl;

    std::string contentDir = "content";
    std::string outputDir = "docs";  // Changed to 'docs' for GitHub Pages
    std::string templatePath = "templates/template.html";

    // Create output directory if it doesn't exist
    if (!fs::exists(outputDir)) {
        fs::create_directory(outputDir);
    }

    // Read template
    std::string templateContent = readFile(templatePath);
    if (templateContent.empty()) {
        std::cerr << "Error: Could not read template file" << std::endl;
        return 1;
    }

    // Process all markdown files
    MarkdownParser parser;
    std::vector<Page> pages;

    for (const auto& entry : fs::directory_iterator(contentDir)) {
        if (entry.path().extension() == ".md") {
            std::string filepath = entry.path().string();
            std::string filename = entry.path().filename().string();
            std::string outputFilename = entry.path().stem().string() + ".html";

            std::cout << "Processing: " << filename << std::endl;

            // Read markdown file
            std::string markdownContent = readFile(filepath);
            if (markdownContent.empty()) {
                continue;
            }

            // Extract title
            std::string title = extractTitle(markdownContent);

            // Convert to HTML
            std::string htmlContent = parser.convertToHTML(markdownContent);

            // Store page info
            Page page;
            page.filename = filename;
            page.title = title;
            page.content = htmlContent;
            page.outputPath = outputFilename;
            pages.push_back(page);
        }
    }

    // Generate HTML files
    for (const auto& page : pages) {
        std::string finalHtml = applyTemplate(templateContent, page.title, page.content, pages);
        std::string outputPath = outputDir + "/" + page.outputPath;
        writeFile(outputPath, finalHtml);
    }

    std::cout << "\n=== Site generation complete! ===" << std::endl;
    std::cout << "Generated " << pages.size() << " pages in '" << outputDir << "' directory" << std::endl;

    return 0;
}
