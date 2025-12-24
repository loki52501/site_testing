#include "template.h"
#include <sstream>
#include <vector>
#include <utility>

std::string applyTemplate(const std::string& templateContent, const std::string& title,
                          const std::string& content, const std::vector<Page>& pages,
                          int subdirectoryDepth, const std::string& toc) {
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

    // Replace TOC placeholder
    pos = result.find("{{TOC}}");
    if (pos != std::string::npos) {
        result.replace(pos, 7, toc);
    }

    // Replace CSS path placeholder - handle different subdirectory depths
    std::string cssPath = "";
    for (int i = 0; i < subdirectoryDepth; i++) {
        cssPath += "../";
    }
    pos = result.find("{{CSS_PATH}}");
    while (pos != std::string::npos) {
        result.replace(pos, 12, cssPath);
        pos = result.find("{{CSS_PATH}}", pos + cssPath.length());
    }

    // Generate navigation - only show main pages in specific order
    std::stringstream nav;

    // Define navigation items (path, display name)
    // Adjust paths based on subdirectory depth
    std::string pathPrefix = "";
    for (int i = 0; i < subdirectoryDepth; i++) {
        pathPrefix += "../";
    }
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
