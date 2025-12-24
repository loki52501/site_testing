#include "listing.h"
#include "category.h"
#include "content_tree.h"
#include "sidebar.h"
#include "template.h"
#include "file_utils.h"
#include <iostream>
#include <sstream>
#include <set>
#include <algorithm>
#include <filesystem>

namespace fs = std::filesystem;

std::string generateBlogListingHTML(const std::vector<BlogPost>& blogPosts, int pageNum,
                                    int postsPerPage, const std::string& category,
                                    const std::string& blogsJsonData) {
    std::stringstream html;

    // Calculate pagination
    int totalPosts = blogPosts.size();
    int totalPages = (totalPosts + postsPerPage - 1) / postsPerPage;
    int startIdx = (pageNum - 1) * postsPerPage;
    int endIdx = std::min(startIdx + postsPerPage, totalPosts);

    // Category-specific titles
    std::string categoryTitle = getCategoryTitle(category);
    std::string categoryDesc = getCategoryDescription(category);

    // Start blog container with three-column layout
    html << "<div class=\"blog-container\">\n";

    // Left Sidebar - Categories (dynamically generated from blog tree)
    std::string blogDir = "content/blog";
    if (fs::exists(blogDir)) {
        ContentNode* blogTree = buildContentTree(blogDir);
        if (blogTree) {
            std::string sidebarHTML = generateSidebar(blogTree, category);
            html << sidebarHTML;
            freeContentTree(blogTree);
        }
    } else {
        // Fallback to hardcoded sidebar
        html << "    <aside class=\"blog-sidebar\">\n";
        html << "        <nav class=\"sidebar-menu\">\n";
        html << "            <h3>Categories</h3>\n";
        html << "            <ul>\n";
        html << "                <li><a href=\"blogs.html\">All Posts</a></li>\n";
        html << "            </ul>\n";
        html << "        </nav>\n";
        html << "    </aside>\n";
    }

    // Main content
    html << "    <div class=\"blog-content\">\n";
    html << "        <h1>" << categoryTitle << "</h1>\n";
    html << "        <p>" << categoryDesc << "</p>\n\n";
    html << "        <div class=\"blog-list\" id=\"search-results\">\n";

    // Show posts for current page
    for (int i = startIdx; i < endIdx; i++) {
        const auto& post = blogPosts[i];
        html << "            <article class=\"blog-item\">\n";

        // Add category/subcategory badge
        std::string badgeText = "";
        if (category.empty() && !post.category.empty()) {
            badgeText = post.category;
            badgeText[0] = std::toupper(badgeText[0]);
        } else if (!category.empty()) {
            std::string path = post.outputPath;
            size_t firstSlash = path.find('/');
            if (firstSlash != std::string::npos) {
                size_t secondSlash = path.find('/', firstSlash + 1);
                if (secondSlash != std::string::npos) {
                    size_t thirdSlash = path.find('/', secondSlash + 1);
                    if (thirdSlash != std::string::npos) {
                        badgeText = path.substr(secondSlash + 1, thirdSlash - secondSlash - 1);
                        badgeText = formatDisplayName(badgeText);
                    }
                }
            }
        }

        if (!badgeText.empty()) {
            html << "                <span class=\"category-badge\">" << badgeText << "</span>\n";
        }

        // Determine correct path
        std::string postPath;
        if (post.outputPath.find("blog/") == 0) {
            postPath = post.outputPath;
        } else if (!post.category.empty() && post.outputPath.find(post.category + "/") == 0) {
            postPath = "blog/" + post.outputPath;
        } else {
            postPath = post.category.empty() ? "blog/" + post.outputPath : "blog/" + post.category + "/" + post.outputPath;
        }

        html << "                <h2><a href=\"" << postPath << "\">" << post.title << "</a></h2>\n";
        html << "                <p class=\"blog-date\">Published on " << post.publishDate << "</p>\n";

        if (!post.excerpt.empty()) {
            html << "                <p class=\"blog-excerpt\">" << post.excerpt << "</p>\n";
        }

        html << "                <a href=\"" << postPath << "\" class=\"read-more\">Read more →</a>\n";
        html << "            </article>\n";
    }

    html << "        </div>\n";

    // Add pagination controls
    if (totalPages > 1) {
        html << "\n<nav class=\"pagination\">\n";
        std::string basePage = category.empty() ? "blogs" : category;

        // Previous button
        if (pageNum > 1) {
            std::string prevPage = (pageNum == 2) ? basePage + ".html" : basePage + "-" + std::to_string(pageNum - 1) + ".html";
            html << "    <a href=\"" << prevPage << "\" class=\"pagination-btn\">&larr; Previous</a>\n";
        } else {
            html << "    <span class=\"pagination-btn disabled\">&larr; Previous</span>\n";
        }

        // Page numbers
        html << "    <div class=\"pagination-numbers\">\n";
        for (int i = 1; i <= totalPages; i++) {
            std::string pageLink = (i == 1) ? basePage + ".html" : basePage + "-" + std::to_string(i) + ".html";
            if (i == pageNum) {
                html << "        <span class=\"page-number active\">" << i << "</span>\n";
            } else {
                html << "        <a href=\"" << pageLink << "\" class=\"page-number\">" << i << "</a>\n";
            }
        }
        html << "    </div>\n";

        // Next button
        if (pageNum < totalPages) {
            std::string nextPage = basePage + "-" + std::to_string(pageNum + 1) + ".html";
            html << "    <a href=\"" << nextPage << "\" class=\"pagination-btn\">Next &rarr;</a>\n";
        } else {
            html << "    <span class=\"pagination-btn disabled\">Next &rarr;</span>\n";
        }

        html << "</nav>\n";
    }

    html << "    </div>\n";

    // Right Sidebar - Search
    html << "    <aside class=\"search-sidebar\">\n";
    html << "        <div class=\"search-sidebar-sticky\">\n";
    html << "            <h3>Search & Filter</h3>\n";
    html << "            <input type=\"text\" id=\"blog-search\" placeholder=\"Search posts...\" aria-label=\"Search blogs\">\n";
    html << "            \n";
    html << "            <div class=\"search-filters\">\n";
    html << "                <h4>Filter by Category</h4>\n";
    html << "                <div class=\"category-filters\">\n";

    // Dynamically generate category filters
    std::set<std::string> uniqueCategories;
    for (const auto& post : blogPosts) {
        if (!post.category.empty()) {
            uniqueCategories.insert(post.category);
        }
    }
    for (const auto& cat : uniqueCategories) {
        std::string displayName = getCategoryTitle(cat);
        size_t blogPos = displayName.find(" Blog");
        if (blogPos != std::string::npos) displayName = displayName.substr(0, blogPos);
        size_t reviewsPos = displayName.find(" Reviews");
        if (reviewsPos != std::string::npos) displayName = displayName.substr(0, reviewsPos);
        size_t thoughtsPos = displayName.find(" Thoughts");
        if (thoughtsPos != std::string::npos) displayName = displayName.substr(0, thoughtsPos);
        html << "                    <a href=\"#\" class=\"category-filter\" data-category=\"" << cat << "\">" << displayName << "</a>\n";
    }

    html << "                </div>\n";
    html << "            </div>\n";
    html << "            \n";
    html << "            <div class=\"sort-control\">\n";
    html << "                <label for=\"blog-sort\">Sort by</label>\n";
    html << "                <select id=\"blog-sort\">\n";
    html << "                    <option value=\"date-desc\">Newest First</option>\n";
    html << "                    <option value=\"date-asc\">Oldest First</option>\n";
    html << "                    <option value=\"title-asc\">Title (A-Z)</option>\n";
    html << "                    <option value=\"title-desc\">Title (Z-A)</option>\n";
    html << "                </select>\n";
    html << "            </div>\n";
    html << "            \n";
    html << "            <p id=\"result-count\" class=\"result-count\"></p>\n";
    html << "        </div>\n";
    html << "    </aside>\n";

    html << "</div>\n";

    // Embed JSON data inline
    if (!blogsJsonData.empty()) {
        html << "<script>\n";
        html << "// Embedded blog data to avoid CORS issues\n";
        html << "window.BLOG_DATA = " << blogsJsonData << ";\n";
        html << "</script>\n";
    }

    html << "<script src=\"search.js\"></script>\n";

    return html.str();
}

void generateCategoryListingPage(ContentNode* categoryNode,
                                  const std::string& outputBaseDir,
                                  const std::string& templateContent,
                                  const std::vector<NavigationItem>& navItems,
                                  const std::string& blogsJsonData) {
    if (!categoryNode || categoryNode->type != NODE_DIRECTORY) return;

    // Collect all posts in this category
    std::vector<ContentNode*> fileNodes;
    collectFileNodes(categoryNode, fileNodes);

    if (fileNodes.empty()) return;

    // Convert to BlogPost objects
    std::vector<BlogPost> posts;
    for (ContentNode* fileNode : fileNodes) {
        BlogPost post;
        post.filename = fileNode->name;
        post.title = fileNode->title;
        post.excerpt = fileNode->excerpt;
        post.publishDate = fileNode->publishDate;
        post.timestamp = fileNode->timestamp;
        post.outputPath = fileNode->outputPath;

        if (fileNode->parent && fileNode->parent->name != "blog") {
            post.category = fileNode->parent->name;
        } else {
            post.category = "";
        }

        posts.push_back(post);
    }

    // Sort by date (newest first)
    std::sort(posts.begin(), posts.end(), [](const BlogPost& a, const BlogPost& b) {
        return a.timestamp > b.timestamp;
    });

    // Generate listing HTML
    std::string listingHTML = generateBlogListingHTML(posts, 1, 10, categoryNode->name, blogsJsonData);

    // Apply template
    std::string title = categoryNode->displayName + " - Blog";
    std::string finalHTML = applyTemplate(templateContent, title, listingHTML, {}, 0, "");

    // Determine output path
    std::string outputPath = outputBaseDir + "/" + categoryNode->name + ".html";

    // Write file
    writeFile(outputPath, finalHTML);

    std::cout << "Generated listing page: " << categoryNode->name << ".html" << std::endl;
}
