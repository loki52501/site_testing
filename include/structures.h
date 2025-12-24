#ifndef STRUCTURES_H
#define STRUCTURES_H

#include <string>
#include <vector>
#include <ctime>

// Page structure for regular content pages
struct Page {
    std::string filename;
    std::string title;
    std::string content;
    std::string outputPath;
};

// Blog post structure
struct BlogPost {
    std::string filename;
    std::string title;
    std::string content;
    std::string excerpt;
    std::string outputPath;
    std::string publishDate;
    std::time_t timestamp;
    std::string category;
};

// Cached metadata for incremental builds
struct CachedMetadata {
    size_t contentHash;
    std::string publishDate;
    std::time_t timestamp;
    std::time_t fileModTime;
};

// Node types for content tree
enum NodeType {
    NODE_FILE,
    NODE_DIRECTORY
};

// Content tree node for hierarchical content structure
struct ContentNode {
    std::string name;
    std::string displayName;
    std::string path;
    std::string outputPath;
    NodeType type;
    int depth;

    // File metadata
    std::string title;
    std::string content;
    std::string excerpt;
    std::string publishDate;
    std::time_t timestamp;

    // Directory tree
    std::vector<ContentNode*> children;
    ContentNode* parent;
    bool hasIndexFile;

    // Constructor
    ContentNode() : type(NODE_FILE), depth(0), timestamp(0), parent(nullptr), hasIndexFile(false) {}
};

// Navigation item for dynamic navbar generation
struct NavigationItem {
    std::string displayName;
    std::string url;
    int order;

    NavigationItem() : order(0) {}
};

// Section configuration for listing pages
struct SectionConfig {
    std::string sectionName;
    std::string displayName;
    bool enableListing;
    bool enableSearch;
    bool enableSidebar;
    bool sortByDate;
    int postsPerPage;
    bool showExcerpts;

    SectionConfig() : enableListing(true), enableSearch(false), enableSidebar(false),
                     sortByDate(false), postsPerPage(10), showExcerpts(true) {}
};

#endif // STRUCTURES_H
