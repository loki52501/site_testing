#ifndef CATEGORY_H
#define CATEGORY_H

#include <string>
#include <vector>

// Get display title for a category
std::string getCategoryTitle(const std::string& category);

// Get description for a category
std::string getCategoryDescription(const std::string& category);

// Discover all categories in blog directory
std::vector<std::string> discoverCategories(const std::string& blogDir);

#endif // CATEGORY_H
