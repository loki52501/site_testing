#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <vector>
#include <algorithm>
#include <functional>

// Module includes
#include "structures.h"
#include "file_utils.h"
#include "metadata.h"
#include "cache.h"
#include "toc.h"
#include "category.h"
#include "content_tree.h"
#include "navigation.h"
#include "sidebar.h"
#include "template.h"
#include "listing.h"
#include "processor.h"

// Existing parsers
#include "markdown_parser.h"
#include "blog_database.h"
#include "jupyter_parser.h"

namespace fs = std::filesystem;

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
    if (!fs::exists(notebooksImagesDir)) {
        fs::create_directories(notebooksImagesDir);
    }
    if (!fs::exists(notebooksImagesOutputDir)) {
        fs::create_directories(notebooksImagesOutputDir);
    }

    // Copy CSS file
    try {
        fs::copy_file(cssSourcePath, cssOutputPath, fs::copy_options::overwrite_existing);
        std::cout << "Copied: " << cssOutputPath << std::endl;
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Error copying CSS file: " << e.what() << std::endl;
    }

    // Copy search.js file
    std::string jsSourcePath = "templates/search.js";
    std::string jsOutputPath = "docs/search.js";
    try {
        fs::copy_file(jsSourcePath, jsOutputPath, fs::copy_options::overwrite_existing);
        std::cout << "Copied: " << jsOutputPath << std::endl;
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Error copying search.js file: " << e.what() << std::endl;
    }

    // Copy images
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

    // Copy CNAME file
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

    // Hash template content
    std::string templateHash = std::to_string(hashString(templateContent));

    MarkdownParser parser;
    std::vector<Page> pages;
    std::vector<BlogPost> blogPosts;

    // Track which files need regeneration
    std::vector<Page> pagesToGenerate;
    int skippedPages = 0;

    // Process regular pages
    for (const auto& entry : fs::directory_iterator(contentDir)) {
        if (entry.is_directory()) continue;

        if (entry.path().extension() == ".md") {
            std::string filepath = entry.path().string();
            std::string filename = entry.path().filename().string();
            std::string outputFilename = entry.path().stem().string() + ".html";
            std::string outputPath = outputDir + "/" + outputFilename;

            std::string markdownContent = readFile(filepath);
            if (markdownContent.empty()) continue;

            std::string title = extractTitle(markdownContent);

            Page page;
            page.filename = filename;
            page.title = title;
            page.outputPath = outputFilename;

            if (!needsPageRegeneration(filepath, outputPath, cache)) {
                std::cout << "Skipping (up-to-date): " << filename << std::endl;
                skippedPages++;
                newCache[filepath] = cache[filepath];
            } else {
                std::cout << "Processing page: " << filename << std::endl;
                std::string htmlContent = parser.convertToHTML(markdownContent);
                page.content = htmlContent;
                pagesToGenerate.push_back(page);

                CachedMetadata metadata;
                metadata.contentHash = 0;
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
    std::vector<std::string> categories = discoverCategories(blogDir);
    for (const auto& categoryName : categories) {
        std::string categoryOutputDir = blogOutputDir + "/" + categoryName;
        if (!fs::exists(categoryOutputDir)) {
            fs::create_directory(categoryOutputDir);
        }

        std::string categoryDir = blogDir + "/" + categoryName;
        processCategoryBlogs(categoryDir, categoryOutputDir, categoryName, blogPosts, blogsToGenerate,
                            skippedBlogs, cache, newCache, templateHash, parser);
    }

    // Process subdirectories recursively
    std::cout << "\n[INFO] Processing subdirectories..." << std::endl;

    if (fs::exists(blogDir)) {
        ContentNode* blogTree = buildContentTree(blogDir);
        if (blogTree) {
            std::vector<NavigationItem> emptyNav;

            // Process the entire blog tree recursively
            processContentNode(blogTree, "docs", templateContent, parser, emptyNav, cache, newCache, templateHash);

            // Generate listing pages for all subdirectories
            std::cout << "\n[INFO] Generating listing pages for subdirectories..." << std::endl;
            std::function<void(ContentNode*)> generateSubdirListings = [&](ContentNode* node) {
                if (!node || node->type != NODE_DIRECTORY) return;

                if (!node->children.empty()) {
                    generateCategoryListingPage(node, "docs", templateContent, emptyNav, "");
                }

                for (ContentNode* child : node->children) {
                    if (child->type == NODE_DIRECTORY) {
                        generateSubdirListings(child);
                    }
                }
            };

            for (ContentNode* child : blogTree->children) {
                if (child->type == NODE_DIRECTORY) {
                    generateSubdirListings(child);
                }
            }

            freeContentTree(blogTree);
            std::cout << "[INFO] Subdirectory processing complete!" << std::endl;
        }
    }

    // Process uncategorized blogs
    if (fs::exists(blogDir)) {
        for (const auto& entry : fs::directory_iterator(blogDir)) {
            if (entry.is_directory()) continue;

            std::string extension = entry.path().extension().string();
            if (extension == ".md" || extension == ".ipynb") {
                std::string filepath = entry.path().string();
                std::string filename = entry.path().filename().string();
                std::string outputFilename = entry.path().stem().string() + ".html";
                std::string outputPath = blogOutputDir + "/" + outputFilename;

                std::string fileContent = readFile(filepath);
                if (fileContent.empty()) continue;

                std::string title;
                std::string excerpt;
                std::string htmlContent;
                std::vector<std::string> extractedImages;

                if (extension == ".ipynb") {
                    JupyterParser jupyterParser;
                    htmlContent = jupyterParser.convertToHTML(fileContent, filepath, extractedImages);

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

                    excerpt = "Jupyter notebook";

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
                post.category = "";

                if (!needsBlogRegeneration(filepath, fileContent, outputPath, templateHash, cache)) {
                    std::cout << "Skipping (up-to-date): " << filename << " [uncategorized]" << std::endl;
                    skippedBlogs++;
                    newCache[filepath] = cache[filepath];
                } else {
                    std::cout << "Processing blog: " << filename << " [uncategorized]" << std::endl;

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

    // Sort blog posts by date
    std::sort(blogPosts.begin(), blogPosts.end(), [](const BlogPost& a, const BlogPost& b) {
        return a.timestamp > b.timestamp;
    });

    // Initialize SQLite database and export to JSON
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

            if (post.category.empty()) {
                entry.url = "blog/" + post.outputPath;
            } else {
                entry.url = "blog/" + post.category + "/" + post.outputPath;
            }

            blogDB.insertBlog(entry);
        }

        blogDB.exportToJSON(jsonPath);
    } else {
        std::cerr << "Warning: Failed to initialize blog database." << std::endl;
    }

    // Read JSON for embedding
    std::string blogsJsonData;
    std::ifstream jsonFile(jsonPath);
    if (jsonFile.is_open()) {
        std::stringstream buffer;
        buffer << jsonFile.rdbuf();
        blogsJsonData = buffer.str();
        jsonFile.close();
    }

    const int POSTS_PER_PAGE = 5;

    // Generate main blog listing page
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

    // Generate category-specific blog listing pages
    for (const auto& category : categories) {
        std::vector<BlogPost> categoryPosts;
        for (const auto& post : blogPosts) {
            if (post.category == category) {
                categoryPosts.push_back(post);
            }
        }

        if (categoryPosts.empty()) continue;

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

    // Generate HTML files for pages
    for (const auto& page : pagesToGenerate) {
        std::string finalHtml = applyTemplate(templateContent, page.title, page.content, pages);
        std::string outputPath = outputDir + "/" + page.outputPath;
        writeFile(outputPath, finalHtml);
    }

    // Generate HTML files for blog posts
    for (const auto& post : blogsToGenerate) {
        int depth = post.category.empty() ? 1 : 2;

        std::string tocHtml = "";
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

        std::string outputPath;
        if (!post.category.empty()) {
            outputPath = blogOutputDir + "/" + post.category + "/" + post.outputPath;
        } else {
            outputPath = blogOutputDir + "/" + post.outputPath;
        }

        writeFile(outputPath, finalHtml);
    }

    // Save cache
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
