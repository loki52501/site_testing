#ifndef LISTING_H
#define LISTING_H

#include "structures.h"
#include <string>
#include <vector>

// Generate blog listing HTML page with pagination and sidebars
std::string generateBlogListingHTML(const std::vector<BlogPost>& blogPosts, int pageNum,
                                    int postsPerPage, const std::string& category = "",
                                    const std::string& blogsJsonData = "");

// Generate listing page for a category/subcategory
void generateCategoryListingPage(ContentNode* categoryNode,
                                  const std::string& outputBaseDir,
                                  const std::string& templateContent,
                                  const std::vector<NavigationItem>& navItems,
                                  const std::string& blogsJsonData = "");

#endif // LISTING_H
