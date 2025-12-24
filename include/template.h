#ifndef TEMPLATE_H
#define TEMPLATE_H

#include "structures.h"
#include <string>
#include <vector>

// Apply template to content with placeholders
std::string applyTemplate(const std::string& templateContent, const std::string& title,
                          const std::string& content, const std::vector<Page>& pages,
                          int subdirectoryDepth = 0, const std::string& toc = "");

#endif // TEMPLATE_H
