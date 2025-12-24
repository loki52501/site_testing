#include "sidebar.h"
#include <sstream>

std::string renderSidebarCategory(ContentNode* category,
                                  const std::string& currentPath,
                                  int nestLevel) {
    std::stringstream html;

    // Determine if this category is active
    std::string categoryUrl = category->name + ".html";
    bool isActive = currentPath.find(category->name) != std::string::npos;

    // Indent based on nesting level
    std::string indent(nestLevel * 4, ' ');

    html << indent << "    <li class=\"category-item\">\n";

    // Category link
    html << indent << "        <a href=\"" << categoryUrl << "\""
         << (isActive ? " class=\"active\"" : "") << ">"
         << category->displayName << "</a>\n";

    // If has subcategories, render nested list
    bool hasSubcategories = false;
    for (ContentNode* child : category->children) {
        if (child->type == NODE_DIRECTORY) {
            hasSubcategories = true;
            break;
        }
    }

    if (hasSubcategories) {
        html << indent << "        <ul class=\"subcategory-list\">\n";
        for (ContentNode* child : category->children) {
            if (child->type == NODE_DIRECTORY) {
                html << renderSidebarCategory(child, currentPath, nestLevel + 1);
            }
        }
        html << indent << "        </ul>\n";
    }

    html << indent << "    </li>\n";

    return html.str();
}

std::string generateSidebar(ContentNode* sectionNode,
                            const std::string& currentPath) {
    std::stringstream html;

    html << "<aside class=\"blog-sidebar\">\n";
    html << "    <nav class=\"sidebar-menu\">\n";
    html << "        <h3>Categories</h3>\n";
    html << "        <ul>\n";

    // Add "All Posts" link
    // Special case: "blog" folder should link to "blogs.html"
    std::string allPostsUrl = (sectionNode->name == "blog") ? "blogs.html" : sectionNode->name + ".html";
    bool isAllActive = currentPath.empty() || currentPath == allPostsUrl;
    html << "            <li><a href=\"" << allPostsUrl << "\""
         << (isAllActive ? " class=\"active\"" : "")
         << ">All Posts</a></li>\n";

    // Recursively render category tree
    for (ContentNode* child : sectionNode->children) {
        if (child->type == NODE_DIRECTORY) {
            html << renderSidebarCategory(child, currentPath, 1);
        }
    }

    html << "        </ul>\n";
    html << "    </nav>\n";
    html << "</aside>\n";

    return html.str();
}
