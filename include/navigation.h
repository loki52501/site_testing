#ifndef NAVIGATION_H
#define NAVIGATION_H

#include "structures.h"
#include <string>
#include <vector>

// Generate navigation items from content tree
std::vector<NavigationItem> generateNavigation(ContentNode* root);

// Render navigation HTML from navigation items
std::string renderNavigation(const std::vector<NavigationItem>& navItems,
                             int subdirectoryDepth);

#endif // NAVIGATION_H
