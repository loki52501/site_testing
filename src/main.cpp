#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <ctime>
#include <iomanip>
#include <functional>
#include "../include/markdown_parser.h"
#include "../include/blog_database.h"
#include "../include/jupyter_parser.h"

namespace fs = std::filesystem;

struct Page {
    std::string filename;
    std::string title;
    std::string content;
    std::string outputPath;
};

struct BlogPost {
    std::string filename;
    std::string title;
    std::string content;
    std::string excerpt;
    std::string outputPath;
    std::string publishDate;
    std::time_t timestamp;
    std::string category;  // tech, movies, or random
};

struct CachedMetadata {
    size_t contentHash;
    std::string publishDate;
    std::time_t timestamp;
    std::time_t fileModTime;  // File modification time for change detection
};

// Node types for content tree
enum NodeType {
    NODE_FILE,
    NODE_DIRECTORY
};

// Content tree node for hierarchical content structure
struct ContentNode {
    std::string name;                           // Folder/file name (e.g., "tech", "about.md")
    std::string displayName;                    // Human-readable name (e.g., "Tech", "About")
    std::string path;                           // Relative path from content root
    std::string outputPath;                     // HTML output path
    NodeType type;                              // FILE or DIRECTORY
    int depth;                                  // Depth in tree (0 = root)

    // File metadata
    std::string title;                          // Extracted from markdown
    std::string content;                        // HTML content
    std::string excerpt;                        // For listing pages
    std::string publishDate;                    // For sorting
    std::time_t timestamp;                      // For sorting

    // Directory tree
    std::vector<ContentNode*> children;         // Child nodes
    ContentNode* parent;                        // Parent node
    bool hasIndexFile;                          // Whether index.md exists

    // Constructor
    ContentNode() : type(NODE_FILE), depth(0), timestamp(0), parent(nullptr), hasIndexFile(false) {}
};

// Navigation item for dynamic navbar generation
struct NavigationItem {
    std::string displayName;                    // "Blog", "About"
    std::string url;                            // "blogs.html", "about.html"
    int order;                                  // For sorting

    // Constructor
    NavigationItem() : order(0) {}
};

// Section configuration for listing pages
struct SectionConfig {
    std::string sectionName;                    // "blogs", "projects", "photos"
    std::string displayName;                    // "Blog", "Projects", "Photos"
    bool enableListing;                         // Generate listing pages
    bool enableSearch;                          // Enable search functionality
    bool enableSidebar;                         // Show category sidebar
    bool sortByDate;                            // Sort by date vs alphabetical
    int postsPerPage;                           // Pagination count
    bool showExcerpts;                          // Show excerpts on listings

    // Constructor
    SectionConfig() : enableListing(true), enableSearch(false), enableSidebar(false),
                     sortByDate(false), postsPerPage(10), showExcerpts(true) {}
};

std::string readFile(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filepath << std::endl;
        return "";
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

void writeFile(const std::string& filepath, const std::string& content) {
    std::ofstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Error: Could not write to file " << filepath << std::endl;
        return;
    }

    file << content;
    file.close();
    std::cout << "Generated: " << filepath << std::endl;
}

std::string extractTitle(const std::string& markdown) {
    std::stringstream ss(markdown);
    std::string line;

    while (std::getline(ss, line)) {
        if (line.length() > 2 && line[0] == '#' && line[1] == ' ') {
            return line.substr(2);
        }
    }

    return "Untitled";
}

std::string extractExcerpt(const std::string& markdown, size_t maxLength = 200) {
    std::stringstream ss(markdown);
    std::string line;
    std::string excerpt;
    bool skippedHeader = false;
    bool skippedSecondLine = false;

    while (std::getline(ss, line)) {
        // Skip lines starting with # (headers)
        if (!line.empty() && line[0] == '#') {
            skippedHeader = true;
            continue;
        }

        // Skip empty lines before we find content
        if (line.empty()) {
            continue;
        }

        // Skip the second line (first non-empty line after header)
        if (skippedHeader && !skippedSecondLine) {
            skippedSecondLine = true;
            continue;
        }

        // Now accumulate paragraph text
        if (!excerpt.empty()) {
            excerpt += " ";
        }
        excerpt += line;

        // Stop if we have enough text
        if (excerpt.length() >= maxLength) {
            break;
        }
    }

    // Truncate to maxLength and add ellipsis if needed
    if (excerpt.length() > maxLength) {
        excerpt = excerpt.substr(0, maxLength);
        size_t lastSpace = excerpt.find_last_of(" ");
        if (lastSpace != std::string::npos) {
            excerpt = excerpt.substr(0, lastSpace);
        }
        excerpt += "...";
    }

    return excerpt;
}

// Generate table of contents from markdown headings
std::string generateTOC(const std::string& markdown) {
    std::stringstream ss(markdown);
    std::stringstream toc;
    std::string line;
    bool hasHeadings = false;

    toc << "<nav class=\"toc\">\n";
    toc << "    <h3>Table of Contents</h3>\n";
    toc << "    <ul>\n";

    while (std::getline(ss, line)) {
        if (!line.empty() && line[0] == '#') {
            // Count heading level
            size_t level = 0;
            while (level < line.length() && line[level] == '#') {
                level++;
            }

            // Skip the first h1 (title) - we don't want it in TOC
            if (level == 1 && !hasHeadings) {
                hasHeadings = true;
                continue;
            }

            // Extract heading text
            std::string headingText = line.substr(level);
            // Trim leading/trailing spaces
            size_t start = headingText.find_first_not_of(" \t");
            size_t end = headingText.find_last_not_of(" \t");
            if (start != std::string::npos && end != std::string::npos) {
                headingText = headingText.substr(start, end - start + 1);
            }

            // Create anchor from heading text (lowercase, replace spaces with hyphens)
            std::string anchor = headingText;
            std::transform(anchor.begin(), anchor.end(), anchor.begin(), ::tolower);
            std::replace(anchor.begin(), anchor.end(), ' ', '-');
            // Remove special characters
            anchor.erase(std::remove_if(anchor.begin(), anchor.end(),
                [](char c) { return !std::isalnum(c) && c != '-'; }), anchor.end());

            // Add indentation based on heading level
            std::string indent = "";
            for (size_t i = 2; i < level; i++) {
                indent += "    ";
            }

            toc << indent << "        <li class=\"toc-level-" << level << "\">"
                << "<a href=\"#" << anchor << "\">" << headingText << "</a></li>\n";
            hasHeadings = true;
        }
    }

    toc << "    </ul>\n";
    toc << "</nav>\n";

    return hasHeadings ? toc.str() : "";
}

std::string getFileModificationDate(const std::string& filepath) {
    auto ftime = fs::last_write_time(filepath);
    auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
        ftime - fs::file_time_type::clock::now() + std::chrono::system_clock::now()
    );
    std::time_t cftime = std::chrono::system_clock::to_time_t(sctp);

    std::stringstream ss;
    ss << std::put_time(std::localtime(&cftime), "%B %d, %Y at %I:%M %p");
    return ss.str();
}

std::time_t getFileModificationTimestamp(const std::string& filepath) {
    auto ftime = fs::last_write_time(filepath);
    auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
        ftime - fs::file_time_type::clock::now() + std::chrono::system_clock::now()
    );
    return std::chrono::system_clock::to_time_t(sctp);
}

// Simple hash function for content
size_t hashString(const std::string& str) {
    return std::hash<std::string>{}(str);
}

// Load cache from file
std::map<std::string, CachedMetadata> loadCache(const std::string& cacheFile) {
    std::map<std::string, CachedMetadata> cache;
    std::ifstream file(cacheFile);
    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            std::stringstream ss(line);
            std::string filepath;
            CachedMetadata metadata;

            // Read: filepath hash timestamp fileModTime publishDate
            ss >> filepath >> metadata.contentHash >> metadata.timestamp >> metadata.fileModTime;

            // Read the rest as publish date (may contain spaces)
            std::getline(ss, metadata.publishDate);
            // Trim leading space
            if (!metadata.publishDate.empty() && metadata.publishDate[0] == ' ') {
                metadata.publishDate = metadata.publishDate.substr(1);
            }

            cache[filepath] = metadata;
        }
        file.close();
    }
    return cache;
}

// Save cache to file
void saveCache(const std::string& cacheFile, const std::map<std::string, CachedMetadata>& cache) {
    std::ofstream file(cacheFile);
    if (file.is_open()) {
        for (const auto& entry : cache) {
            file << entry.first << " "
                 << entry.second.contentHash << " "
                 << entry.second.timestamp << " "
                 << entry.second.fileModTime << " "
                 << entry.second.publishDate << "\n";
        }
        file.close();
    }
}

// Check if page needs regeneration based on file modification time
bool needsPageRegeneration(const std::string& sourcePath, const std::string& outputPath,
                           const std::map<std::string, CachedMetadata>& cache) {
    // If output doesn't exist, needs regeneration
    if (!fs::exists(outputPath)) {
        return true;
    }

    // Get current file modification time
    std::time_t currentModTime = getFileModificationTimestamp(sourcePath);

    // Check if cached modification time exists and matches
    auto it = cache.find(sourcePath);
    if (it == cache.end() || it->second.fileModTime != currentModTime) {
        return true; // File modified or no cache entry
    }

    return false; // File unchanged
}

// Check if blog post needs regeneration based on content hash
bool needsBlogRegeneration(const std::string& sourcePath, const std::string& sourceContent,
                           const std::string& outputPath, const std::string& templateHash,
                           const std::map<std::string, CachedMetadata>& cache) {
    // If output doesn't exist, needs regeneration
    if (!fs::exists(outputPath)) {
        return true;
    }

    // Calculate hash of current content
    size_t currentHash = hashString(sourceContent + templateHash);

    // Check if cached hash exists and matches
    auto it = cache.find(sourcePath);
    if (it == cache.end() || it->second.contentHash != currentHash) {
        return true; // Content changed or no cache entry
    }

    return false; // Content unchanged
}

// ==================== Content Tree Building Functions ====================

/**
 * Format display name from folder/file name
 * Examples: "tech" -> "Tech", "my-projects" -> "My Projects", "about.md" -> "About"
 */
std::string formatDisplayName(const std::string& name) {
    std::string display = name;

    // Remove extension if present
    size_t dotPos = display.find_last_of('.');
    if (dotPos != std::string::npos) {
        display = display.substr(0, dotPos);
    }

    // Replace hyphens/underscores with spaces
    std::replace(display.begin(), display.end(), '-', ' ');
    std::replace(display.begin(), display.end(), '_', ' ');

    // Capitalize first letter of each word
    bool capitalize = true;
    for (char& c : display) {
        if (capitalize && std::isalpha(c)) {
            c = std::toupper(c);
            capitalize = false;
        } else if (c == ' ') {
            capitalize = true;
        }
    }

    return display;
}

/**
 * Calculate output HTML path based on position in tree
 */
std::string calculateOutputPath(ContentNode* node) {
    std::vector<std::string> pathParts;

    ContentNode* current = node;
    while (current) {
        if (current->type == NODE_FILE) {
            // For files, use stem (filename without extension) + .html
            std::string filename = fs::path(current->name).stem().string();
            pathParts.insert(pathParts.begin(), filename + ".html");
        } else {
            // For directories, add the folder name (including root)
            pathParts.insert(pathParts.begin(), current->name);
        }
        current = current->parent;
    }

    // Join path parts
    std::string result;
    for (size_t i = 0; i < pathParts.size(); i++) {
        if (i > 0) result += "/";
        result += pathParts[i];
    }

    return result;
}

/**
 * Recursively build content tree from directory structure
 *
 * @param rootPath Starting directory (e.g., "content")
 * @param parent Parent node (nullptr for root)
 * @param depth Current depth in tree
 * @return Root ContentNode with complete tree
 */
ContentNode* buildContentTree(const std::string& rootPath,
                               ContentNode* parent = nullptr,
                               int depth = 0) {
    ContentNode* node = new ContentNode();
    node->path = rootPath;
    node->depth = depth;
    node->parent = parent;

    if (fs::is_directory(rootPath)) {
        node->type = NODE_DIRECTORY;
        node->name = fs::path(rootPath).filename().string();
        node->displayName = formatDisplayName(node->name);

        // Check for index.md
        std::string indexPath = rootPath + "/index.md";
        node->hasIndexFile = fs::exists(indexPath);

        // Collect and sort entries
        std::vector<fs::path> entries;
        for (const auto& entry : fs::directory_iterator(rootPath)) {
            std::string filename = entry.path().filename().string();
            // Skip hidden files/folders and special directories
            if (filename[0] == '.' || filename == "images") {
                continue;
            }
            entries.push_back(entry.path());
        }

        // Sort entries alphabetically
        std::sort(entries.begin(), entries.end());

        // Recursively process children
        for (const auto& entry : entries) {
            ContentNode* child = buildContentTree(
                entry.string(), node, depth + 1
            );
            if (child) {
                node->children.push_back(child);
            }
        }
    } else if (fs::is_regular_file(rootPath)) {
        std::string extension = fs::path(rootPath).extension().string();
        if (extension == ".md" || extension == ".ipynb") {
            node->type = NODE_FILE;
            node->name = fs::path(rootPath).filename().string();

            // Read and parse file (basic metadata extraction)
            std::string fileContent = readFile(rootPath);
            if (!fileContent.empty()) {
                if (extension == ".ipynb") {
                    // Extract title from first markdown cell in notebook
                    try {
                        json notebook = json::parse(fileContent);
                        bool foundTitle = false;
                        if (notebook.contains("cells")) {
                            for (const auto& cell : notebook["cells"]) {
                                if (cell["cell_type"] == "markdown" && cell.contains("source")) {
                                    std::string source;
                                    if (cell["source"].is_string()) {
                                        source = cell["source"];
                                    } else if (cell["source"].is_array() && !cell["source"].empty()) {
                                        source = cell["source"][0];
                                    }
                                    if (source.find("# ") == 0) {
                                        node->title = source.substr(2);
                                        // Remove newline
                                        if (!node->title.empty() && node->title.back() == '\n') {
                                            node->title.pop_back();
                                        }
                                        foundTitle = true;
                                        break;
                                    }
                                }
                            }
                        }
                        if (!foundTitle) {
                            node->title = fs::path(rootPath).stem().string();
                        }
                    } catch (...) {
                        node->title = fs::path(rootPath).stem().string();
                    }
                    node->displayName = node->title;
                    node->excerpt = "Jupyter notebook";
                } else {
                    // Markdown file
                    node->title = extractTitle(fileContent);
                    node->displayName = node->title;
                    node->excerpt = extractExcerpt(fileContent);
                }
                node->publishDate = getFileModificationDate(rootPath);
                node->timestamp = getFileModificationTimestamp(rootPath);
            }

            // Calculate output path
            node->outputPath = calculateOutputPath(node);
        } else {
            // Not a markdown or notebook file, skip it
            delete node;
            return nullptr;
        }
    }

    return node;
}

/**
 * Free content tree memory recursively
 */
void freeContentTree(ContentNode* root) {
    if (!root) return;

    for (ContentNode* child : root->children) {
        freeContentTree(child);
    }

    delete root;
}

/**
 * Print content tree for debugging
 */
void printTree(ContentNode* node, int indent = 0) {
    if (!node) return;

    std::string indentStr(indent * 2, ' ');
    std::string typeStr = (node->type == NODE_DIRECTORY) ? "[DIR]" : "[FILE]";

    std::cout << indentStr << typeStr << " " << node->name;
    if (node->type == NODE_DIRECTORY && node->hasIndexFile) {
        std::cout << " (has index.md)";
    }
    if (node->type == NODE_FILE && !node->title.empty()) {
        std::cout << " - \"" << node->title << "\"";
    }
    std::cout << std::endl;

    for (ContentNode* child : node->children) {
        printTree(child, indent + 1);
    }
}

// ==================== Navigation Generation Functions ====================

/**
 * Generate navigation items from content tree
 * Only top-level items appear in navbar
 *
 * @param root Root content node
 * @return Vector of navigation items, alphabetically sorted
 */
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

/**
 * Render navigation HTML from navigation items
 *
 * @param navItems Navigation items to render
 * @param subdirectoryDepth Depth for relative path calculation
 * @return HTML string for navigation
 */
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

std::string applyTemplate(const std::string& templateContent, const std::string& title,
                          const std::string& content, const std::vector<Page>& pages, int subdirectoryDepth = 0,
                          const std::string& toc = "") {
    std::string result = templateContent;

    // Replace title placeholder
    size_t pos = result.find("{{TITLE}}");
    while (pos != std::string::npos) {
        result.replace(pos, 9, title);
        pos = result.find("{{TITLE}}", pos + title.length());
    }

    // Replace content placeholder
    pos = result.find("{{CONTENT}}");
    if (pos != std::string::npos) {
        result.replace(pos, 11, content);
    }

    // Replace TOC placeholder
    pos = result.find("{{TOC}}");
    if (pos != std::string::npos) {
        result.replace(pos, 7, toc);
    }

    // Replace CSS path placeholder - handle different subdirectory depths
    std::string cssPath = "";
    for (int i = 0; i < subdirectoryDepth; i++) {
        cssPath += "../";
    }
    pos = result.find("{{CSS_PATH}}");
    while (pos != std::string::npos) {
        result.replace(pos, 12, cssPath);
        pos = result.find("{{CSS_PATH}}", pos + cssPath.length());
    }

    // Generate navigation - only show main pages in specific order
    std::stringstream nav;

    // Define navigation items (path, display name)
    // Adjust paths based on subdirectory depth
    std::string pathPrefix = "";
    for (int i = 0; i < subdirectoryDepth; i++) {
        pathPrefix += "../";
    }
    std::vector<std::pair<std::string, std::string>> navItems = {
        {pathPrefix + "index.html", "Home"},
        {pathPrefix + "about.html", "About"},
        {pathPrefix + "projects.html", "Projects"},
        {pathPrefix + "blogs.html", "Blog"}
    };

    for (const auto& item : navItems) {
        nav << "<a href=\"" << item.first << "\">" << item.second << "</a>";
    }

    pos = result.find("{{NAV}}");
    if (pos != std::string::npos) {
        result.replace(pos, 7, nav.str());
    }

    return result;
}

// ==================== Sidebar Generation Functions ====================

/**
 * Recursively render a sidebar category with nesting
 *
 * @param category Category node to render
 * @param currentPath Current page path for highlighting
 * @param nestLevel Nesting level for indentation
 * @return HTML string for this category and its children
 */
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

/**
 * Generate hierarchical sidebar HTML for a section
 * Shows all subcategories with expand/collapse functionality
 *
 * @param sectionNode Directory node for current section
 * @param currentPath Current page path (to highlight active item)
 * @return HTML string for sidebar
 */
std::string generateSidebar(ContentNode* sectionNode,
                            const std::string& currentPath = "") {
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

// Helper function to get category display title
std::string getCategoryTitle(const std::string& category) {
    if (category.empty()) return "All Posts";
    if (category == "tech") return "Tech Blog";
    if (category == "movies") return "Movie Reviews";
    if (category == "books") return "Books, My Readings";
    if (category == "random") return "Random Thoughts";
    // Default: format the category name nicely
    return formatDisplayName(category);
}

// Helper function to get category description
std::string getCategoryDescription(const std::string& category) {
    if (category.empty()) return "All my blog posts";
    if (category == "tech") return "Technology, programming, and software development";
    if (category == "movies") return "Movie reviews, analysis, and recommendations";
    if (category == "books") return "My book reviews, notes, and literary explorations";
    if (category == "random") return "Random musings and miscellaneous topics";
    // Default: generic description
    return "Posts in the " + formatDisplayName(category) + " category";
}

std::string generateBlogListingHTML(const std::vector<BlogPost>& blogPosts, int pageNum, int postsPerPage, const std::string& category = "", const std::string& blogsJsonData = "") {
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
    // Build blog tree to discover all categories and subcategories
    std::string blogDir = "content/blog";
    if (fs::exists(blogDir)) {
        ContentNode* blogTree = buildContentTree(blogDir);
        if (blogTree) {
            // Generate dynamic sidebar showing all categories and subcategories
            std::string sidebarHTML = generateSidebar(blogTree, category);
            html << sidebarHTML;
            freeContentTree(blogTree);
        }
    } else {
        // Fallback to hardcoded sidebar if blog directory doesn't exist
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
                // Showing all posts - display main category
                badgeText = post.category;
                badgeText[0] = std::toupper(badgeText[0]);
            } else if (!category.empty()) {
                // Showing specific category - check if post is from a subcategory
                // Extract subcategory from outputPath (e.g., blog/tech/AI/post.html -> AI)
                std::string path = post.outputPath;
                size_t firstSlash = path.find('/');
                if (firstSlash != std::string::npos) {
                    size_t secondSlash = path.find('/', firstSlash + 1);
                    if (secondSlash != std::string::npos) {
                        size_t thirdSlash = path.find('/', secondSlash + 1);
                        if (thirdSlash != std::string::npos) {
                            // Has subcategory (e.g., blog/tech/AI/post.html)
                            badgeText = path.substr(secondSlash + 1, thirdSlash - secondSlash - 1);
                            // Format display name
                            badgeText = formatDisplayName(badgeText);
                        }
                    }
                }
            }

            if (!badgeText.empty()) {
                html << "                <span class=\"category-badge\">" << badgeText << "</span>\n";
            }

            // Determine correct path
            // If outputPath already starts with "blog/", use it as-is (new tree-based system)
            // Otherwise, construct path the old way (for backward compatibility)
            std::string postPath;
            if (post.outputPath.find("blog/") == 0) {
                // New system: outputPath already has full path like "blog/tech/AI/post.html"
                postPath = post.outputPath;
            } else if (!post.category.empty() && post.outputPath.find(post.category + "/") == 0) {
                // OutputPath already starts with category (e.g., "tech/AI/post.html")
                postPath = "blog/" + post.outputPath;
            } else {
                // Old system: construct from category + filename
                postPath = post.category.empty() ? "blog/" + post.outputPath : "blog/" + post.category + "/" + post.outputPath;
            }

            html << "                <h2><a href=\"" << postPath << "\">" << post.title << "</a></h2>\n";
            html << "                <p class=\"blog-date\">Published on " << post.publishDate << "</p>\n";

            // Add excerpt if available
            if (!post.excerpt.empty()) {
                html << "                <p class=\"blog-excerpt\">" << post.excerpt << "</p>\n";
            }

            html << "                <a href=\"" << postPath << "\" class=\"read-more\">Read more →</a>\n";
            html << "            </article>\n";
        }

    html << "        </div>\n"; // close blog-list

    // Add pagination controls if there are multiple pages
    if (totalPages > 1) {
        html << "\n<nav class=\"pagination\">\n";

        // Determine base page name from category
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

    html << "    </div>\n"; // close blog-content

    // Right Sidebar - Search
    html << "    <aside class=\"search-sidebar\">\n";
    html << "        <div class=\"search-sidebar-sticky\">\n";
    html << "            <h3>Search & Filter</h3>\n";
    html << "            <input type=\"text\" id=\"blog-search\" placeholder=\"Search posts...\" aria-label=\"Search blogs\">\n";
    html << "            \n";
    html << "            <div class=\"search-filters\">\n";
    html << "                <h4>Filter by Category</h4>\n";
    html << "                <div class=\"category-filters\">\n";

    // Dynamically generate category filters from all posts
    std::set<std::string> uniqueCategories;
    for (const auto& post : blogPosts) {
        if (!post.category.empty()) {
            uniqueCategories.insert(post.category);
        }
    }
    for (const auto& cat : uniqueCategories) {
        std::string displayName = getCategoryTitle(cat);
        // Remove " Blog" or similar suffixes from display name for cleaner filter labels
        size_t blogPos = displayName.find(" Blog");
        if (blogPos != std::string::npos) {
            displayName = displayName.substr(0, blogPos);
        }
        size_t reviewsPos = displayName.find(" Reviews");
        if (reviewsPos != std::string::npos) {
            displayName = displayName.substr(0, reviewsPos);
        }
        size_t thoughtsPos = displayName.find(" Thoughts");
        if (thoughtsPos != std::string::npos) {
            displayName = displayName.substr(0, thoughtsPos);
        }
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

    html << "</div>\n"; // close blog-container

    // Embed JSON data inline if provided (avoids CORS issues with file:// protocol)
    if (!blogsJsonData.empty()) {
        html << "<script>\n";
        html << "// Embedded blog data to avoid CORS issues\n";
        html << "window.BLOG_DATA = " << blogsJsonData << ";\n";
        html << "</script>\n";
    }

    html << "<script src=\"search.js\"></script>\n";

    return html.str();
}

// ==================== Tree-Based Content Processing ====================

/**
 * Collect all file nodes from a directory (including nested subdirectories)
 *
 * @param node Directory node to collect from
 * @param files Vector to store collected file nodes
 */
void collectFileNodes(ContentNode* node, std::vector<ContentNode*>& files) {
    if (!node) return;

    if (node->type == NODE_FILE) {
        files.push_back(node);
    } else if (node->type == NODE_DIRECTORY) {
        for (ContentNode* child : node->children) {
            collectFileNodes(child, files);
        }
    }
}

/**
 * Generate listing page for a category/subcategory
 *
 * @param categoryNode Directory node for the category
 * @param outputBaseDir Base output directory (e.g., "docs")
 * @param templateContent Template HTML
 * @param navItems Navigation items
 * @param blogsJsonData JSON data for search (empty for now)
 */
void generateCategoryListingPage(ContentNode* categoryNode,
                                  const std::string& outputBaseDir,
                                  const std::string& templateContent,
                                  const std::vector<NavigationItem>& navItems,
                                  const std::string& blogsJsonData = "") {
    if (!categoryNode || categoryNode->type != NODE_DIRECTORY) return;

    // Collect all posts in this category (including nested)
    std::vector<ContentNode*> fileNodes;
    collectFileNodes(categoryNode, fileNodes);

    if (fileNodes.empty()) {
        return; // No posts, skip listing page
    }

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

        // Determine category from parent
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

/**
 * Recursively process content tree node and generate HTML files
 * This handles subdirectories at any depth
 *
 * @param node Current node to process
 * @param outputBaseDir Base output directory (e.g., "docs")
 * @param templateContent Template HTML content
 * @param parser Markdown parser
 * @param navItems Navigation items for all pages
 * @param cache Build cache
 * @param newCache Updated cache
 * @param templateHash Template hash for cache validation
 */
void processContentNode(ContentNode* node,
                       const std::string& outputBaseDir,
                       const std::string& templateContent,
                       MarkdownParser& parser,
                       const std::vector<NavigationItem>& navItems,
                       const std::map<std::string, CachedMetadata>& cache,
                       std::map<std::string, CachedMetadata>& newCache,
                       const std::string& templateHash) {
    if (!node) return;

    if (node->type == NODE_FILE) {
        // Process markdown or jupyter notebook file
        std::string outputPath = outputBaseDir + "/" + node->outputPath;

        // Create output directory if needed
        fs::path outputFilePath(outputPath);
        fs::path outputDirPath = outputFilePath.parent_path();
        if (!outputDirPath.empty() && !fs::exists(outputDirPath)) {
            fs::create_directories(outputDirPath);
        }

        // Read file content
        std::string fileContent = readFile(node->path);
        if (fileContent.empty()) {
            return;
        }

        // Check if regeneration is needed
        bool needsRegen = needsBlogRegeneration(node->path, fileContent, outputPath, templateHash, cache);

        if (!needsRegen) {
            std::cout << "Skipping (up-to-date): " << node->outputPath << std::endl;

            // Update cache with existing metadata
            auto cachedIt = cache.find(node->path);
            if (cachedIt != cache.end()) {
                newCache[node->path] = cachedIt->second;
            }
            return;
        }

        // Determine file type and convert to HTML
        std::string htmlContent;
        std::string toc = "";
        fs::path filePath(node->path);
        std::string extension = filePath.extension().string();

        if (extension == ".ipynb") {
            // Process Jupyter notebook
            JupyterParser jupyterParser;
            std::vector<std::string> extractedImages;
            htmlContent = jupyterParser.convertToHTML(fileContent, node->path, extractedImages);

            // Copy extracted images to docs/images/notebooks/
            for (const auto& imagePath : extractedImages) {
                fs::path imgPath(imagePath);
                std::string destPath = "docs/images/notebooks/" + imgPath.filename().string();
                try {
                    fs::copy_file(imagePath, destPath, fs::copy_options::overwrite_existing);
                } catch (const fs::filesystem_error& e) {
                    std::cerr << "Error copying image: " << e.what() << std::endl;
                }
            }
        } else {
            // Process markdown
            htmlContent = parser.convertToHTML(fileContent);
            toc = generateTOC(fileContent);
        }

        // Calculate subdirectory depth for relative paths
        // Count slashes in outputPath to determine depth
        // e.g., "blog/tech/AI/post.html" has 3 slashes = depth 3
        int depth = 0;
        for (char c : node->outputPath) {
            if (c == '/') depth++;
        }

        // Apply template
        std::string finalHTML = applyTemplate(templateContent, node->title, htmlContent,
                                             {}, depth, toc);

        // Write output file
        writeFile(outputPath, finalHTML);

        // Update cache
        CachedMetadata metadata;
        metadata.contentHash = hashString(fileContent + templateHash);
        metadata.publishDate = node->publishDate;
        metadata.timestamp = node->timestamp;
        metadata.fileModTime = getFileModificationTimestamp(node->path);
        newCache[node->path] = metadata;

    } else if (node->type == NODE_DIRECTORY) {
        // Recursively process all children
        for (ContentNode* child : node->children) {
            processContentNode(child, outputBaseDir, templateContent, parser,
                             navItems, cache, newCache, templateHash);
        }
    }
}

// Helper function to process blog posts from a specific category directory
// NOW INCLUDES SUBDIRECTORIES RECURSIVELY
void processCategoryBlogs(const std::string& categoryDir, const std::string& categoryOutputDir,
                         const std::string& categoryName, std::vector<BlogPost>& allBlogPosts,
                         std::vector<BlogPost>& blogsToGenerate, int& skippedBlogs,
                         const std::map<std::string, CachedMetadata>& cache,
                         std::map<std::string, CachedMetadata>& newCache,
                         const std::string& templateHash, MarkdownParser& parser) {
    if (!fs::exists(categoryDir)) {
        return;
    }

    // Build tree for this category to get ALL posts including nested ones
    ContentNode* categoryTree = buildContentTree(categoryDir);
    if (!categoryTree) {
        return;
    }

    // Collect all file nodes recursively
    std::vector<ContentNode*> fileNodes;
    collectFileNodes(categoryTree, fileNodes);

    // Process each file
    for (ContentNode* fileNode : fileNodes) {
        if (fileNode->type != NODE_FILE) continue;

        std::string filepath = fileNode->path;
        std::string filename = fileNode->name;

        // Determine output path - for files in subdirectories, it will be like blog/tech/AI/post.html
        std::string outputPath = "docs/" + fileNode->outputPath;

        std::string markdownContent = readFile(filepath);
        if (markdownContent.empty()) {
            continue;
        }

        std::string title = fileNode->title;
        std::string excerpt = fileNode->excerpt;

        // Use cached date if available
        std::string publishDate;
        std::time_t timestamp;

        auto cachedIt = cache.find(filepath);
        if (cachedIt != cache.end() && !cachedIt->second.publishDate.empty()) {
            publishDate = cachedIt->second.publishDate;
            timestamp = cachedIt->second.timestamp;
        } else {
            publishDate = fileNode->publishDate;
            timestamp = fileNode->timestamp;
        }

        BlogPost post;
        post.filename = filename;
        post.title = title;
        post.excerpt = excerpt;
        post.outputPath = fileNode->outputPath; // Full path like blog/tech/AI/post.html
        post.publishDate = publishDate;
        post.timestamp = timestamp;
        post.category = categoryName;

        // Check if regeneration is needed (already done by processContentNode, so skip for now)
        // We're just collecting metadata for listing pages here
        allBlogPosts.push_back(post);

        // Update cache
        CachedMetadata metadata;
        metadata.contentHash = hashString(markdownContent + templateHash);
        metadata.publishDate = publishDate;
        metadata.timestamp = timestamp;
        metadata.fileModTime = getFileModificationTimestamp(filepath);
        newCache[filepath] = metadata;
    }

    freeContentTree(categoryTree);
}

// OLD VERSION - KEPT FOR REFERENCE BUT NOT USED
void processCategoryBlogs_OLD(const std::string& categoryDir, const std::string& categoryOutputDir,
                         const std::string& categoryName, std::vector<BlogPost>& allBlogPosts,
                         std::vector<BlogPost>& blogsToGenerate, int& skippedBlogs,
                         const std::map<std::string, CachedMetadata>& cache,
                         std::map<std::string, CachedMetadata>& newCache,
                         const std::string& templateHash, MarkdownParser& parser) {
    if (!fs::exists(categoryDir)) {
        return;
    }

    for (const auto& entry : fs::directory_iterator(categoryDir)) {
        if (entry.path().extension() == ".md") {
            std::string filepath = entry.path().string();
            std::string filename = entry.path().filename().string();
            std::string outputFilename = entry.path().stem().string() + ".html";
            std::string outputPath = categoryOutputDir + "/" + outputFilename;

            std::string markdownContent = readFile(filepath);
            if (markdownContent.empty()) {
                continue;
            }

            std::string title = extractTitle(markdownContent);
            std::string excerptMarkdown = extractExcerpt(markdownContent);
            std::string excerpt = parser.convertToHTML(excerptMarkdown);

            // Use cached date if available, otherwise get from file system
            std::string publishDate;
            std::time_t timestamp;

            auto cachedIt = cache.find(filepath);
            if (cachedIt != cache.end() && !cachedIt->second.publishDate.empty()) {
                publishDate = cachedIt->second.publishDate;
                timestamp = cachedIt->second.timestamp;
            } else {
                publishDate = getFileModificationDate(filepath);
                timestamp = getFileModificationTimestamp(filepath);
            }

            BlogPost post;
            post.filename = filename;
            post.title = title;
            post.excerpt = excerpt;
            post.outputPath = outputFilename;
            post.publishDate = publishDate;
            post.timestamp = timestamp;
            post.category = categoryName;

            // Check if regeneration is needed
            if (!needsBlogRegeneration(filepath, markdownContent, outputPath, templateHash, cache)) {
                std::cout << "Skipping (up-to-date): " << filename << " [" << categoryName << "]" << std::endl;
                skippedBlogs++;
                newCache[filepath] = cache.at(filepath);
            } else {
                std::cout << "Processing blog: " << filename << " [" << categoryName << "]" << std::endl;
                std::string htmlContent = parser.convertToHTML(markdownContent);
                post.content = htmlContent;
                blogsToGenerate.push_back(post);

                CachedMetadata metadata;
                metadata.contentHash = hashString(markdownContent + templateHash);
                metadata.publishDate = publishDate;
                metadata.timestamp = timestamp;
                metadata.fileModTime = 0;
                newCache[filepath] = metadata;
            }

            allBlogPosts.push_back(post);
        }
    }
}

int main(int argc, char* argv[]) {
    std::cout << "=== Markdown Static Site Generator ===" << std::endl;

    std::string contentDir = "content";
    std::string blogDir = "content/blog";
    std::string imagesDir = "content/images";
    std::string notebooksImagesDir = "content/images/notebooks";
    std::string outputDir = "docs";
    std::string blogOutputDir = "docs/blog";
    std::string imagesOutputDir = "docs/images";
    std::string notebooksImagesOutputDir = "docs/images/notebooks";
    std::string templatePath = "templates/template.html";
    std::string cssSourcePath = "templates/style.css";
    std::string cssOutputPath = "docs/style.css";
    std::string cacheFile = ".build_cache";

    // Create output directories if they don't exist
    if (!fs::exists(outputDir)) {
        fs::create_directory(outputDir);
    }
    if (!fs::exists(blogOutputDir)) {
        fs::create_directory(blogOutputDir);
    }
    if (!fs::exists(imagesOutputDir)) {
        fs::create_directory(imagesOutputDir);
    }
    // Create notebooks subdirectory in content/images
    if (!fs::exists(notebooksImagesDir)) {
        fs::create_directories(notebooksImagesDir);
    }
    // Create notebooks subdirectory in docs/images
    if (!fs::exists(notebooksImagesOutputDir)) {
        fs::create_directories(notebooksImagesOutputDir);
    }

    // Copy CSS file to output directory
    try {
        fs::copy_file(cssSourcePath, cssOutputPath, fs::copy_options::overwrite_existing);
        std::cout << "Copied: " << cssOutputPath << std::endl;
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Error copying CSS file: " << e.what() << std::endl;
    }

    // Copy search.js file to output directory
    std::string jsSourcePath = "templates/search.js";
    std::string jsOutputPath = "docs/search.js";
    try {
        fs::copy_file(jsSourcePath, jsOutputPath, fs::copy_options::overwrite_existing);
        std::cout << "Copied: " << jsOutputPath << std::endl;
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Error copying search.js file: " << e.what() << std::endl;
    }

    // Copy images directory to output directory
    if (fs::exists(imagesDir)) {
        try {
            for (const auto& entry : fs::directory_iterator(imagesDir)) {
                if (entry.is_regular_file()) {
                    std::string filename = entry.path().filename().string();
                    std::string destPath = imagesOutputDir + "/" + filename;
                    fs::copy_file(entry.path(), destPath, fs::copy_options::overwrite_existing);
                    std::cout << "Copied image: " << destPath << std::endl;
                }
            }
        } catch (const fs::filesystem_error& e) {
            std::cerr << "Error copying images: " << e.what() << std::endl;
        }
    }

    // Copy CNAME file if it exists (for custom domain on GitHub Pages)
    std::string cnameSource = "CNAME";
    std::string cnameDest = "docs/CNAME";
    if (fs::exists(cnameSource)) {
        try {
            fs::copy_file(cnameSource, cnameDest, fs::copy_options::overwrite_existing);
            std::cout << "Copied: " << cnameDest << std::endl;
        } catch (const fs::filesystem_error& e) {
            std::cerr << "Error copying CNAME file: " << e.what() << std::endl;
        }
    }

    // Load cache
    std::map<std::string, CachedMetadata> cache = loadCache(cacheFile);
    std::map<std::string, CachedMetadata> newCache;

    // Read template
    std::string templateContent = readFile(templatePath);
    if (templateContent.empty()) {
        std::cerr << "Error: Could not read template file" << std::endl;
        return 1;
    }

    // Hash template content for change detection
    std::string templateHash = std::to_string(hashString(templateContent));

    MarkdownParser parser;
    std::vector<Page> pages;
    std::vector<BlogPost> blogPosts;

    // Track which files need regeneration
    std::vector<Page> pagesToGenerate;
    int skippedPages = 0;

    // Process regular pages (not in blog directory)
    for (const auto& entry : fs::directory_iterator(contentDir)) {
        if (entry.is_directory()) {
            continue; // Skip directories
        }

        if (entry.path().extension() == ".md") {
            std::string filepath = entry.path().string();
            std::string filename = entry.path().filename().string();
            std::string outputFilename = entry.path().stem().string() + ".html";
            std::string outputPath = outputDir + "/" + outputFilename;

            // Always read for metadata (needed for navigation)
            std::string markdownContent = readFile(filepath);
            if (markdownContent.empty()) {
                continue;
            }

            std::string title = extractTitle(markdownContent);

            Page page;
            page.filename = filename;
            page.title = title;
            page.outputPath = outputFilename;

            // Check if regeneration is needed based on file modification time
            if (!needsPageRegeneration(filepath, outputPath, cache)) {
                std::cout << "Skipping (up-to-date): " << filename << std::endl;
                skippedPages++;
                // Keep metadata in new cache
                newCache[filepath] = cache[filepath];
            } else {
                std::cout << "Processing page: " << filename << std::endl;
                std::string htmlContent = parser.convertToHTML(markdownContent);
                page.content = htmlContent;
                pagesToGenerate.push_back(page);
                // Store new metadata with file modification time
                CachedMetadata metadata;
                metadata.contentHash = 0;  // Not used for pages
                metadata.publishDate = "";
                metadata.timestamp = 0;
                metadata.fileModTime = getFileModificationTimestamp(filepath);
                newCache[filepath] = metadata;
            }

            pages.push_back(page);
        }
    }

    // Process blog posts from all categories
    std::vector<BlogPost> blogsToGenerate;
    int skippedBlogs = 0;

    // Dynamically discover and process all category directories
    std::vector<std::string> categories;
    if (fs::exists(blogDir)) {
        for (const auto& entry : fs::directory_iterator(blogDir)) {
            if (entry.is_directory()) {
                std::string categoryName = entry.path().filename().string();
                // Skip hidden directories and special directories
                if (categoryName[0] != '.' && categoryName != "images") {
                    categories.push_back(categoryName);

                    // Create output directory for this category if it doesn't exist
                    std::string categoryOutputDir = blogOutputDir + "/" + categoryName;
                    if (!fs::exists(categoryOutputDir)) {
                        fs::create_directory(categoryOutputDir);
                    }

                    // Process blogs in this category
                    std::string categoryDir = blogDir + "/" + categoryName;
                    processCategoryBlogs(categoryDir, categoryOutputDir, categoryName, blogPosts, blogsToGenerate,
                                        skippedBlogs, cache, newCache, templateHash, parser);
                }
            }
        }
    }

    // ==================== NEW: Process subdirectories recursively ====================
    std::cout << "\n[INFO] Processing subdirectories..." << std::endl;

    // Build tree for blog directory to find ALL subdirectories
    if (fs::exists(blogDir)) {
        ContentNode* blogTree = buildContentTree(blogDir);
        if (blogTree) {
            // Generate empty nav items for now (will be used properly in full tree-based system)
            std::vector<NavigationItem> emptyNav;

            // Process the entire blog tree recursively
            // This will generate HTML for files in subdirectories like blog/tech/AI/, blog/tech/Thinking like a engineer/
            processContentNode(blogTree, "docs", templateContent, parser, emptyNav, cache, newCache, templateHash);

            // Generate listing pages for all subdirectories (AI.html, "Thinking like a engineer.html", etc.)
            std::cout << "\n[INFO] Generating listing pages for subdirectories..." << std::endl;
            std::function<void(ContentNode*)> generateSubdirListings = [&](ContentNode* node) {
                if (!node || node->type != NODE_DIRECTORY) return;

                // Generate listing page for this directory (if it has posts)
                if (!node->children.empty()) {
                    generateCategoryListingPage(node, "docs", templateContent, emptyNav, "");
                }

                // Recursively process subdirectories
                for (ContentNode* child : node->children) {
                    if (child->type == NODE_DIRECTORY) {
                        generateSubdirListings(child);
                    }
                }
            };

            // Start from blog tree children (tech, movies, random, and their subdirectories)
            for (ContentNode* child : blogTree->children) {
                if (child->type == NODE_DIRECTORY) {
                    generateSubdirListings(child);
                }
            }

            freeContentTree(blogTree);
            std::cout << "[INFO] Subdirectory processing complete!" << std::endl;
        }
    }
    // ==================================================================================

    // Process uncategorized blogs (directly in content/blog)
    if (fs::exists(blogDir)) {
        for (const auto& entry : fs::directory_iterator(blogDir)) {
            // Skip directories (we already processed them above)
            if (entry.is_directory()) {
                continue;
            }

            std::string extension = entry.path().extension().string();
            if (extension == ".md" || extension == ".ipynb") {
                std::string filepath = entry.path().string();
                std::string filename = entry.path().filename().string();
                std::string outputFilename = entry.path().stem().string() + ".html";
                std::string outputPath = blogOutputDir + "/" + outputFilename;

                std::string fileContent = readFile(filepath);
                if (fileContent.empty()) {
                    continue;
                }

                std::string title;
                std::string excerpt;
                std::string htmlContent;
                std::vector<std::string> extractedImages;

                if (extension == ".ipynb") {
                    // Process Jupyter notebook
                    JupyterParser jupyterParser;
                    htmlContent = jupyterParser.convertToHTML(fileContent, filepath, extractedImages);

                    // Extract title from first markdown cell or use filename
                    try {
                        json notebook = json::parse(fileContent);
                        bool foundTitle = false;
                        if (notebook.contains("cells")) {
                            for (const auto& cell : notebook["cells"]) {
                                if (cell["cell_type"] == "markdown" && cell.contains("source")) {
                                    std::string source;
                                    if (cell["source"].is_string()) {
                                        source = cell["source"];
                                    } else if (cell["source"].is_array() && !cell["source"].empty()) {
                                        source = cell["source"][0];
                                    }
                                    if (source.find("# ") == 0) {
                                        title = source.substr(2);
                                        // Remove newline
                                        if (!title.empty() && title.back() == '\n') {
                                            title.pop_back();
                                        }
                                        foundTitle = true;
                                        break;
                                    }
                                }
                            }
                        }
                        if (!foundTitle) {
                            title = entry.path().stem().string();
                        }
                    } catch (...) {
                        title = entry.path().stem().string();
                    }

                    excerpt = "Jupyter notebook";  // Simple excerpt for notebooks

                    // Copy extracted images to docs/images/notebooks/
                    for (const auto& imagePath : extractedImages) {
                        fs::path imgPath(imagePath);
                        std::string destPath = "docs/images/notebooks/" + imgPath.filename().string();
                        try {
                            fs::copy_file(imagePath, destPath, fs::copy_options::overwrite_existing);
                        } catch (const fs::filesystem_error& e) {
                            std::cerr << "Error copying image: " << e.what() << std::endl;
                        }
                    }
                } else {
                    // Process Markdown
                    title = extractTitle(fileContent);
                    std::string excerptMarkdown = extractExcerpt(fileContent);
                    excerpt = parser.convertToHTML(excerptMarkdown);
                }

                std::string publishDate;
                std::time_t timestamp;

                auto cachedIt = cache.find(filepath);
                if (cachedIt != cache.end() && !cachedIt->second.publishDate.empty()) {
                    publishDate = cachedIt->second.publishDate;
                    timestamp = cachedIt->second.timestamp;
                } else {
                    publishDate = getFileModificationDate(filepath);
                    timestamp = getFileModificationTimestamp(filepath);
                }

                BlogPost post;
                post.filename = filename;
                post.title = title;
                post.excerpt = excerpt;
                post.outputPath = outputFilename;
                post.publishDate = publishDate;
                post.timestamp = timestamp;
                post.category = "";  // No category

                if (!needsBlogRegeneration(filepath, fileContent, outputPath, templateHash, cache)) {
                    std::cout << "Skipping (up-to-date): " << filename << " [uncategorized]" << std::endl;
                    skippedBlogs++;
                    newCache[filepath] = cache[filepath];
                } else {
                    std::cout << "Processing blog: " << filename << " [uncategorized]" << std::endl;

                    // Only convert markdown to HTML if not already done (for .md files)
                    if (extension == ".md") {
                        htmlContent = parser.convertToHTML(fileContent);
                    }

                    post.content = htmlContent;
                    blogsToGenerate.push_back(post);

                    CachedMetadata metadata;
                    metadata.contentHash = hashString(fileContent + templateHash);
                    metadata.publishDate = publishDate;
                    metadata.timestamp = timestamp;
                    metadata.fileModTime = 0;
                    newCache[filepath] = metadata;
                }

                blogPosts.push_back(post);
            }
        }
    }

    // Sort blog posts by date (newest first)
    std::sort(blogPosts.begin(), blogPosts.end(), [](const BlogPost& a, const BlogPost& b) {
        return a.timestamp > b.timestamp;
    });

    // Initialize SQLite database and export to JSON for client-side search
    std::string dbPath = "docs/blogs.db";
    std::string jsonPath = "docs/blogs.json";
    BlogDatabase blogDB(dbPath);

    if (blogDB.initialize()) {
        std::cout << "Populating blog database..." << std::endl;

        for (const auto& post : blogPosts) {
            BlogEntry entry;
            entry.title = post.title;
            entry.excerpt = post.excerpt;
            entry.category = post.category.empty() ? "uncategorized" : post.category;
            entry.publishDate = post.publishDate;
            entry.timestamp = post.timestamp;

            // Generate URL based on category
            if (post.category.empty()) {
                entry.url = "blog/" + post.outputPath;
            } else {
                entry.url = "blog/" + post.category + "/" + post.outputPath;
            }

            blogDB.insertBlog(entry);
        }

        // Export to JSON for client-side search
        blogDB.exportToJSON(jsonPath);
    } else {
        std::cerr << "Warning: Failed to initialize blog database. Search functionality will not be available." << std::endl;
    }

    // Read the JSON file to embed in HTML (avoids CORS issues with file:// protocol)
    std::string blogsJsonData;
    std::ifstream jsonFile(jsonPath);
    if (jsonFile.is_open()) {
        std::stringstream buffer;
        buffer << jsonFile.rdbuf();
        blogsJsonData = buffer.str();
        jsonFile.close();
    }

    const int POSTS_PER_PAGE = 5;

    // Generate main blog listing page (all posts from all categories)
    if (!blogPosts.empty()) {
        int totalPosts = blogPosts.size();
        int totalPages = (totalPosts + POSTS_PER_PAGE - 1) / POSTS_PER_PAGE;

        for (int pageNum = 1; pageNum <= totalPages; pageNum++) {
            std::string blogListingHTML = generateBlogListingHTML(blogPosts, pageNum, POSTS_PER_PAGE, "", blogsJsonData);
            Page blogIndexPage;
            blogIndexPage.filename = "blogs.md";
            blogIndexPage.title = "Blog";
            blogIndexPage.content = blogListingHTML;

            if (pageNum == 1) {
                blogIndexPage.outputPath = "blogs.html";
            } else {
                blogIndexPage.outputPath = "blogs-" + std::to_string(pageNum) + ".html";
            }

            pages.push_back(blogIndexPage);
            pagesToGenerate.push_back(blogIndexPage);
        }
    }

    // Generate category-specific blog listing pages (using dynamically discovered categories)
    for (const auto& category : categories) {
        // Filter posts by category
        std::vector<BlogPost> categoryPosts;
        for (const auto& post : blogPosts) {
            if (post.category == category) {
                categoryPosts.push_back(post);
            }
        }

        if (categoryPosts.empty()) {
            continue;
        }

        // Generate paginated listing for this category
        int totalPosts = categoryPosts.size();
        int totalPages = (totalPosts + POSTS_PER_PAGE - 1) / POSTS_PER_PAGE;

        for (int pageNum = 1; pageNum <= totalPages; pageNum++) {
            std::string categoryListingHTML = generateBlogListingHTML(categoryPosts, pageNum, POSTS_PER_PAGE, category, blogsJsonData);
            Page categoryIndexPage;
            categoryIndexPage.filename = category + ".md";
            categoryIndexPage.title = getCategoryTitle(category);
            categoryIndexPage.content = categoryListingHTML;

            if (pageNum == 1) {
                categoryIndexPage.outputPath = category + ".html";
            } else {
                categoryIndexPage.outputPath = category + "-" + std::to_string(pageNum) + ".html";
            }

            pages.push_back(categoryIndexPage);
            pagesToGenerate.push_back(categoryIndexPage);
        }
    }

    // Generate HTML files only for pages that need updating
    for (const auto& page : pagesToGenerate) {
        std::string finalHtml = applyTemplate(templateContent, page.title, page.content, pages);
        std::string outputPath = outputDir + "/" + page.outputPath;
        writeFile(outputPath, finalHtml);
    }

    // Generate HTML files only for blog posts that need updating
    for (const auto& post : blogsToGenerate) {
        // Determine subdirectory depth based on category
        // Categorized posts (tech/movies/random) are 2 levels deep: blog/category/
        // Uncategorized posts are 1 level deep: blog/
        int depth = post.category.empty() ? 1 : 2;

        // Generate TOC from markdown content for this post
        std::string tocHtml = "";
        // Find the original markdown file to generate TOC
        std::string markdownPath;
        if (!post.category.empty()) {
            markdownPath = blogDir + "/" + post.category + "/" + post.filename;
        } else {
            markdownPath = blogDir + "/" + post.filename;
        }

        if (fs::exists(markdownPath)) {
            std::string markdown = readFile(markdownPath);
            tocHtml = generateTOC(markdown);
        }

        std::string finalHtml = applyTemplate(templateContent, post.title, post.content, pages, depth, tocHtml);

        // Determine output path based on category
        std::string outputPath;
        if (!post.category.empty()) {
            outputPath = blogOutputDir + "/" + post.category + "/" + post.outputPath;
        } else {
            outputPath = blogOutputDir + "/" + post.outputPath;
        }

        writeFile(outputPath, finalHtml);
    }

    // Save cache for next build
    saveCache(cacheFile, newCache);

    std::cout << "\n=== Site generation complete! ===" << std::endl;
    std::cout << "Processed " << pages.size() << " pages (" << pagesToGenerate.size() << " generated, "
              << skippedPages << " skipped)" << std::endl;
    std::cout << "Processed " << blogPosts.size() << " blog posts (" << blogsToGenerate.size() << " generated, "
              << skippedBlogs << " skipped)" << std::endl;

    if (skippedPages > 0 || skippedBlogs > 0) {
        std::cout << "\nIncremental build saved time by skipping " << (skippedPages + skippedBlogs)
                  << " up-to-date files!" << std::endl;
    }

    return 0;
}
