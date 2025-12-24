#ifndef CONTENT_TREE_H
#define CONTENT_TREE_H

#include "structures.h"
#include <string>
#include <vector>

// Format display name from folder/file name
std::string formatDisplayName(const std::string& name);

// Calculate output HTML path based on position in tree
std::string calculateOutputPath(ContentNode* node);

// Recursively build content tree from directory structure
ContentNode* buildContentTree(const std::string& rootPath,
                               ContentNode* parent = nullptr,
                               int depth = 0);

// Free content tree memory recursively
void freeContentTree(ContentNode* root);

// Print content tree for debugging
void printTree(ContentNode* node, int indent = 0);

// Collect all file nodes from a directory (including nested subdirectories)
void collectFileNodes(ContentNode* node, std::vector<ContentNode*>& files);

#endif // CONTENT_TREE_H
