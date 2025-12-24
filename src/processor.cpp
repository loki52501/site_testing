#include "processor.h"
#include "file_utils.h"
#include "metadata.h"
#include "cache.h"
#include "content_tree.h"
#include "template.h"
#include "toc.h"
#include "jupyter_parser.h"
#include <filesystem>
#include <iostream>
#include <algorithm>

namespace fs = std::filesystem;

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
        std::string outputPath = outputBaseDir + "/" + node->outputPath;

        // Create output directory if needed
        fs::path outputFilePath(outputPath);
        fs::path outputDirPath = outputFilePath.parent_path();
        if (!outputDirPath.empty() && !fs::exists(outputDirPath)) {
            fs::create_directories(outputDirPath);
        }

        std::string fileContent = readFile(node->path);
        if (fileContent.empty()) return;

        bool needsRegen = needsBlogRegeneration(node->path, fileContent, outputPath, templateHash, cache);

        if (!needsRegen) {
            std::cout << "Skipping (up-to-date): " << node->outputPath << std::endl;
            auto cachedIt = cache.find(node->path);
            if (cachedIt != cache.end()) {
                newCache[node->path] = cachedIt->second;
            }
            return;
        }

        // Convert to HTML
        std::string htmlContent;
        std::string toc = "";
        fs::path filePath(node->path);
        std::string extension = filePath.extension().string();

        if (extension == ".ipynb") {
            JupyterParser jupyterParser;
            std::vector<std::string> extractedImages;
            htmlContent = jupyterParser.convertToHTML(fileContent, node->path, extractedImages);

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
            htmlContent = parser.convertToHTML(fileContent);
            toc = generateTOC(fileContent);
        }

        // Calculate subdirectory depth
        int depth = 0;
        for (char c : node->outputPath) {
            if (c == '/') depth++;
        }

        std::string finalHTML = applyTemplate(templateContent, node->title, htmlContent,
                                             {}, depth, toc);

        writeFile(outputPath, finalHTML);

        // Update cache
        CachedMetadata metadata;
        metadata.contentHash = hashString(fileContent + templateHash);
        metadata.publishDate = node->publishDate;
        metadata.timestamp = node->timestamp;
        metadata.fileModTime = getFileModificationTimestamp(node->path);
        newCache[node->path] = metadata;

    } else if (node->type == NODE_DIRECTORY) {
        for (ContentNode* child : node->children) {
            processContentNode(child, outputBaseDir, templateContent, parser,
                             navItems, cache, newCache, templateHash);
        }
    }
}

void processCategoryBlogs(const std::string& categoryDir, const std::string& categoryOutputDir,
                         const std::string& categoryName, std::vector<BlogPost>& allBlogPosts,
                         std::vector<BlogPost>& blogsToGenerate, int& skippedBlogs,
                         const std::map<std::string, CachedMetadata>& cache,
                         std::map<std::string, CachedMetadata>& newCache,
                         const std::string& templateHash, MarkdownParser& parser) {
    if (!fs::exists(categoryDir)) return;

    ContentNode* categoryTree = buildContentTree(categoryDir);
    if (!categoryTree) return;

    std::vector<ContentNode*> fileNodes;
    collectFileNodes(categoryTree, fileNodes);

    for (ContentNode* fileNode : fileNodes) {
        if (fileNode->type != NODE_FILE) continue;

        std::string filepath = fileNode->path;
        std::string filename = fileNode->name;
        std::string outputPath = "docs/" + fileNode->outputPath;

        std::string markdownContent = readFile(filepath);
        if (markdownContent.empty()) continue;

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
        post.outputPath = fileNode->outputPath;
        post.publishDate = publishDate;
        post.timestamp = timestamp;
        post.category = categoryName;

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
