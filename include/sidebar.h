#ifndef SIDEBAR_H
#define SIDEBAR_H

#include "structures.h"
#include <string>

// Recursively render a sidebar category with nesting
std::string renderSidebarCategory(ContentNode* category,
                                  const std::string& currentPath,
                                  int nestLevel);

// Generate hierarchical sidebar HTML for a section
std::string generateSidebar(ContentNode* sectionNode,
                            const std::string& currentPath = "");

#endif // SIDEBAR_H
