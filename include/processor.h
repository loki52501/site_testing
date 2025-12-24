#ifndef PROCESSOR_H
#define PROCESSOR_H

#include "structures.h"
#include "markdown_parser.h"
#include <string>
#include <vector>
#include <map>

// Recursively process content tree node and generate HTML files
void processContentNode(ContentNode* node,
                       const std::string& outputBaseDir,
                       const std::string& templateContent,
                       MarkdownParser& parser,
                       const std::vector<NavigationItem>& navItems,
                       const std::map<std::string, CachedMetadata>& cache,
                       std::map<std::string, CachedMetadata>& newCache,
                       const std::string& templateHash);

// Process blog posts from a specific category directory (includes subdirectories)
void processCategoryBlogs(const std::string& categoryDir, const std::string& categoryOutputDir,
                         const std::string& categoryName, std::vector<BlogPost>& allBlogPosts,
                         std::vector<BlogPost>& blogsToGenerate, int& skippedBlogs,
                         const std::map<std::string, CachedMetadata>& cache,
                         std::map<std::string, CachedMetadata>& newCache,
                         const std::string& templateHash, MarkdownParser& parser);

#endif // PROCESSOR_H
