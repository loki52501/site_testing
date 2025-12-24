#include "category.h"
#include "content_tree.h"
#include <filesystem>

namespace fs = std::filesystem;

std::string getCategoryTitle(const std::string& category) {
    if (category.empty()) return "All Posts";
    if (category == "tech") return "Tech Blog";
    if (category == "movies") return "Movie Reviews";
    if (category == "books") return "Books, My Readings";
    if (category == "random") return "Random Thoughts";
    // Default: format the category name nicely
    return formatDisplayName(category);
}

std::string getCategoryDescription(const std::string& category) {
    if (category.empty()) return "All my blog posts";
    if (category == "tech") return "Technology, programming, and software development";
    if (category == "movies") return "Movie reviews, analysis, and recommendations";
    if (category == "books") return "My book reviews, notes, and literary explorations";
    if (category == "random") return "Random musings and miscellaneous topics";
    // Default: generic description
    return "Posts in the " + formatDisplayName(category) + " category";
}

std::vector<std::string> discoverCategories(const std::string& blogDir) {
    std::vector<std::string> categories;

    if (!fs::exists(blogDir)) {
        return categories;
    }

    for (const auto& entry : fs::directory_iterator(blogDir)) {
        if (entry.is_directory()) {
            std::string categoryName = entry.path().filename().string();
            // Skip hidden directories and special directories
            if (categoryName[0] != '.' && categoryName != "images") {
                categories.push_back(categoryName);
            }
        }
    }

    return categories;
}
