#include "content_tree.h"
#include "file_utils.h"
#include "metadata.h"
#include "jupyter_parser.h"
#include <filesystem>
#include <algorithm>
#include <iostream>
#include <cctype>

namespace fs = std::filesystem;

// Forward declaration for JSON parsing
using json = nlohmann::json;

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

ContentNode* buildContentTree(const std::string& rootPath,
                               ContentNode* parent,
                               int depth) {
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

void freeContentTree(ContentNode* root) {
    if (!root) return;

    for (ContentNode* child : root->children) {
        freeContentTree(child);
    }

    delete root;
}

void printTree(ContentNode* node, int indent) {
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
