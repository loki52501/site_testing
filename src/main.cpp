#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <vector>
#include <map>
#include <algorithm>
#include <ctime>
#include <iomanip>
#include "../include/markdown_parser.h"

namespace fs = std::filesystem;

struct Page {
    std::string filename;
    std::string title;
    std::string content;
    std::string outputPath;
};

struct BlogPost {
    std::string filename;
    std::string title;
    std::string content;
    std::string outputPath;
    std::string publishDate;
    std::time_t timestamp;
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

std::string applyTemplate(const std::string& templateContent, const std::string& title,
                          const std::string& content, const std::vector<Page>& pages, bool isInSubdirectory = false) {
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

    // Generate navigation - only show main pages in specific order
    std::stringstream nav;

    // Define navigation items (path, display name)
    // Adjust paths based on whether we're in a subdirectory
    std::string pathPrefix = isInSubdirectory ? "../" : "";
    std::vector<std::pair<std::string, std::string>> navItems = {
        {pathPrefix + "index.html", "Home"},
        {pathPrefix + "about.html", "About"},
        {pathPrefix + "projects.html", "Projects"},
        {pathPrefix + "blogs.html", "Blog"}
    };

    for (const auto& item : navItems) {
        nav << "<a href=\"" << item.first << "\">" << item.second << "</a>";
    }

    pos = result.find("{{NAV}}");
    if (pos != std::string::npos) {
        result.replace(pos, 7, nav.str());
    }

    return result;
}

std::string generateBlogListingHTML(const std::vector<BlogPost>& blogPosts) {
    std::stringstream html;

    html << "<h1>Blog Posts</h1>\n";
    html << "<p>Welcome to my blog. Here are all my posts:</p>\n\n";
    html << "<div class=\"blog-list\">\n";

    for (const auto& post : blogPosts) {
        html << "    <article class=\"blog-item\">\n";
        html << "        <h2><a href=\"blog/" << post.outputPath << "\">" << post.title << "</a></h2>\n";
        html << "        <p class=\"blog-date\">Published on " << post.publishDate << "</p>\n";
        html << "    </article>\n";
    }

    html << "</div>\n";

    return html.str();
}

int main(int argc, char* argv[]) {
    std::cout << "=== Markdown Static Site Generator ===" << std::endl;

    std::string contentDir = "content";
    std::string blogDir = "content/blog";
    std::string outputDir = "docs";
    std::string blogOutputDir = "docs/blog";
    std::string templatePath = "templates/template.html";

    // Create output directories if they don't exist
    if (!fs::exists(outputDir)) {
        fs::create_directory(outputDir);
    }
    if (!fs::exists(blogOutputDir)) {
        fs::create_directory(blogOutputDir);
    }

    // Read template
    std::string templateContent = readFile(templatePath);
    if (templateContent.empty()) {
        std::cerr << "Error: Could not read template file" << std::endl;
        return 1;
    }

    MarkdownParser parser;
    std::vector<Page> pages;
    std::vector<BlogPost> blogPosts;

    // Process regular pages (not in blog directory)
    for (const auto& entry : fs::directory_iterator(contentDir)) {
        if (entry.is_directory()) {
            continue; // Skip directories
        }

        if (entry.path().extension() == ".md") {
            std::string filepath = entry.path().string();
            std::string filename = entry.path().filename().string();
            std::string outputFilename = entry.path().stem().string() + ".html";

            std::cout << "Processing page: " << filename << std::endl;

            std::string markdownContent = readFile(filepath);
            if (markdownContent.empty()) {
                continue;
            }

            std::string title = extractTitle(markdownContent);
            std::string htmlContent = parser.convertToHTML(markdownContent);

            Page page;
            page.filename = filename;
            page.title = title;
            page.content = htmlContent;
            page.outputPath = outputFilename;
            pages.push_back(page);
        }
    }

    // Process blog posts
    if (fs::exists(blogDir)) {
        for (const auto& entry : fs::directory_iterator(blogDir)) {
            if (entry.path().extension() == ".md") {
                std::string filepath = entry.path().string();
                std::string filename = entry.path().filename().string();
                std::string outputFilename = entry.path().stem().string() + ".html";

                std::cout << "Processing blog: " << filename << std::endl;

                std::string markdownContent = readFile(filepath);
                if (markdownContent.empty()) {
                    continue;
                }

                std::string title = extractTitle(markdownContent);
                std::string htmlContent = parser.convertToHTML(markdownContent);
                std::string publishDate = getFileModificationDate(filepath);
                std::time_t timestamp = getFileModificationTimestamp(filepath);

                BlogPost post;
                post.filename = filename;
                post.title = title;
                post.content = htmlContent;
                post.outputPath = outputFilename;
                post.publishDate = publishDate;
                post.timestamp = timestamp;
                blogPosts.push_back(post);
            }
        }
    }

    // Sort blog posts by date (newest first)
    std::sort(blogPosts.begin(), blogPosts.end(), [](const BlogPost& a, const BlogPost& b) {
        return a.timestamp > b.timestamp;
    });

    // Generate blog listing page and add to pages
    std::string blogListingHTML = generateBlogListingHTML(blogPosts);
    Page blogIndexPage;
    blogIndexPage.filename = "blogs.md";
    blogIndexPage.title = "Blog";
    blogIndexPage.content = blogListingHTML;
    blogIndexPage.outputPath = "blogs.html";
    pages.push_back(blogIndexPage);

    // Generate HTML files for regular pages
    for (const auto& page : pages) {
        std::string finalHtml = applyTemplate(templateContent, page.title, page.content, pages);
        std::string outputPath = outputDir + "/" + page.outputPath;
        writeFile(outputPath, finalHtml);
    }

    // Generate HTML files for blog posts
    for (const auto& post : blogPosts) {
        std::string finalHtml = applyTemplate(templateContent, post.title, post.content, pages, true);
        std::string outputPath = blogOutputDir + "/" + post.outputPath;
        writeFile(outputPath, finalHtml);
    }

    std::cout << "\n=== Site generation complete! ===" << std::endl;
    std::cout << "Generated " << pages.size() << " pages and " << blogPosts.size()
              << " blog posts in '" << outputDir << "' directory" << std::endl;

    return 0;
}
