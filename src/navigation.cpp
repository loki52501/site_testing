#include "navigation.h"
#include <sstream>
#include <algorithm>

std::vector<NavigationItem> generateNavigation(ContentNode* root) {
    std::vector<NavigationItem> navItems;

    if (!root) return navItems;

    for (ContentNode* child : root->children) {
        NavigationItem item;

        if (child->type == NODE_FILE) {
            // Single file at root level (e.g., about.md)
            item.displayName = child->displayName;
            item.url = child->outputPath;
        } else if (child->type == NODE_DIRECTORY) {
            // Folder becomes navbar item
            item.displayName = child->displayName;

            // Determine URL
            if (child->hasIndexFile) {
                // Links to index.html in that folder
                item.url = child->name + "/index.html";
            } else {
                // Links to auto-generated listing page
                item.url = child->name + ".html";
            }
        }

        navItems.push_back(item);
    }

    // Sort alphabetically by display name
    std::sort(navItems.begin(), navItems.end(),
              [](const NavigationItem& a, const NavigationItem& b) {
                  return a.displayName < b.displayName;
              });

    return navItems;
}

std::string renderNavigation(const std::vector<NavigationItem>& navItems,
                             int subdirectoryDepth) {
    std::stringstream nav;

    // Calculate path prefix based on depth
    std::string pathPrefix = "";
    for (int i = 0; i < subdirectoryDepth; i++) {
        pathPrefix += "../";
    }

    for (const auto& item : navItems) {
        nav << "<a href=\"" << pathPrefix << item.url << "\">"
            << item.displayName << "</a>";
    }

    return nav.str();
}
