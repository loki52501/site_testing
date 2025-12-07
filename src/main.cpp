#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <vector>
#include <map>
#include <algorithm>
#include <ctime>
#include <iomanip>
#include <functional>
#include "../include/markdown_parser.h"

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
};

struct CachedMetadata {
    size_t contentHash;
    std::string publishDate;
    std::time_t timestamp;
    std::time_t fileModTime;  // File modification time for change detection
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
    bool foundFirstParagraph = false;

    while (std::getline(ss, line)) {
        // Skip empty lines and headings
        if (!line.empty() || !(line.length() > 0 && line[0] == '#')) {
      

        // Accumulate paragraph text
        if (!line.empty()) {
            if (!excerpt.empty()) {
                excerpt += " ";
            }
            excerpt += line;
            foundFirstParagraph = true;

            // Stop if we have enough text
            if (excerpt.length() >= maxLength) {
                break;
            }
        } else if (foundFirstParagraph) {
            // Stop at the end of the first paragraph
            break;
        }
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

std::string applyTemplate(const std::string& templateContent, const std::string& title,
                          const std::string& content, const std::vector<Page>& pages, bool isInSubdirectory = false) {
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

    // Replace CSS path placeholder
    std::string cssPath = isInSubdirectory ? "../" : "";
    pos = result.find("{{CSS_PATH}}");
    while (pos != std::string::npos) {
        result.replace(pos, 12, cssPath);
        pos = result.find("{{CSS_PATH}}", pos + cssPath.length());
    }

    // Generate navigation - only show main pages in specific order
    std::stringstream nav;

    // Define navigation items (path, display name)
    // Adjust paths based on whether we're in a subdirectory
    std::string pathPrefix = isInSubdirectory ? "../" : "";
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

std::string generateBlogListingHTML(const std::vector<BlogPost>& blogPosts, int pageNum, int postsPerPage) {
    std::stringstream html;

    // Calculate pagination
    int totalPosts = blogPosts.size();
    int totalPages = (totalPosts + postsPerPage - 1) / postsPerPage;
    int startIdx = (pageNum - 1) * postsPerPage;
    int endIdx = std::min(startIdx + postsPerPage, totalPosts);

    html << "<h1>Blog Posts</h1>\n";
    html << "<p>Welcome to my blog. Here are all my posts:</p>\n\n";
    html << "<div class=\"blog-list\">\n";

    // Show posts for current page
    for (int i = startIdx; i < endIdx; i++) {
        const auto& post = blogPosts[i];
        html << "    <article class=\"blog-item\">\n";
        html << "        <h2><a href=\"blog/" << post.outputPath << "\">" << post.title << "</a></h2>\n";
        html << "        <p class=\"blog-date\">Published on " << post.publishDate << "</p>\n";

        // Add excerpt if available
        if (!post.excerpt.empty()) {
            html << "        <p class=\"blog-excerpt\">" << post.excerpt << "</p>\n";
        }

        html << "        <a href=\"blog/" << post.outputPath << "\" class=\"read-more\">Read more →</a>\n";
        html << "    </article>\n";
    }

    html << "</div>\n";

    // Add pagination controls if there are multiple pages
    if (totalPages > 1) {
        html << "\n<nav class=\"pagination\">\n";

        // Previous button
        if (pageNum > 1) {
            std::string prevPage = (pageNum == 2) ? "blogs.html" : "blogs-" + std::to_string(pageNum - 1) + ".html";
            html << "    <a href=\"" << prevPage << "\" class=\"pagination-btn\">&larr; Previous</a>\n";
        } else {
            html << "    <span class=\"pagination-btn disabled\">&larr; Previous</span>\n";
        }

        // Page numbers
        html << "    <div class=\"pagination-numbers\">\n";
        for (int i = 1; i <= totalPages; i++) {
            std::string pageLink = (i == 1) ? "blogs.html" : "blogs-" + std::to_string(i) + ".html";
            if (i == pageNum) {
                html << "        <span class=\"page-number active\">" << i << "</span>\n";
            } else {
                html << "        <a href=\"" << pageLink << "\" class=\"page-number\">" << i << "</a>\n";
            }
        }
        html << "    </div>\n";

        // Next button
        if (pageNum < totalPages) {
            std::string nextPage = "blogs-" + std::to_string(pageNum + 1) + ".html";
            html << "    <a href=\"" << nextPage << "\" class=\"pagination-btn\">Next &rarr;</a>\n";
        } else {
            html << "    <span class=\"pagination-btn disabled\">Next &rarr;</span>\n";
        }

        html << "</nav>\n";
    }

    return html.str();
}

int main(int argc, char* argv[]) {
    std::cout << "=== Markdown Static Site Generator ===" << std::endl;

    std::string contentDir = "content";
    std::string blogDir = "content/blog";
    std::string imagesDir = "content/images";
    std::string outputDir = "docs";
    std::string blogOutputDir = "docs/blog";
    std::string imagesOutputDir = "docs/images";
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

    // Copy CSS file to output directory
    try {
        fs::copy_file(cssSourcePath, cssOutputPath, fs::copy_options::overwrite_existing);
        std::cout << "Copied: " << cssOutputPath << std::endl;
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Error copying CSS file: " << e.what() << std::endl;
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

    // Process blog posts
    std::vector<BlogPost> blogsToGenerate;
    int skippedBlogs = 0;
    if (fs::exists(blogDir)) {
        for (const auto& entry : fs::directory_iterator(blogDir)) {
            if (entry.path().extension() == ".md") {
                std::string filepath = entry.path().string();
                std::string filename = entry.path().filename().string();
                std::string outputFilename = entry.path().stem().string() + ".html";
                std::string outputPath = blogOutputDir + "/" + outputFilename;

                std::string markdownContent = readFile(filepath);
                if (markdownContent.empty()) {
                    continue;
                }

                std::string title = extractTitle(markdownContent);
                std::string excerpt = extractExcerpt(markdownContent);

                // Use cached date if available, otherwise get from file system
                std::string publishDate;
                std::time_t timestamp;

                auto cachedIt = cache.find(filepath);
                if (cachedIt != cache.end() && !cachedIt->second.publishDate.empty()) {
                    // Use cached date (preserves original publish date)
                    publishDate = cachedIt->second.publishDate;
                    timestamp = cachedIt->second.timestamp;
                } else {
                    // First time seeing this file, get date from file system
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

                // Check if regeneration is needed based on content hash (to preserve dates)
                if (!needsBlogRegeneration(filepath, markdownContent, outputPath, templateHash, cache)) {
                    std::cout << "Skipping (up-to-date): " << filename << std::endl;
                    skippedBlogs++;
                    // Keep metadata in new cache
                    newCache[filepath] = cache[filepath];
                } else {
                    std::cout << "Processing blog: " << filename << std::endl;
                    std::string htmlContent = parser.convertToHTML(markdownContent);
                    post.content = htmlContent;
                    blogsToGenerate.push_back(post);
                    // Store new metadata with preserved date
                    CachedMetadata metadata;
                    metadata.contentHash = hashString(markdownContent + templateHash);
                    metadata.publishDate = publishDate;
                    metadata.timestamp = timestamp;
                    metadata.fileModTime = 0;  // Not used for blogs
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

    // Generate paginated blog listing pages
    const int POSTS_PER_PAGE = 5;
    int totalPosts = blogPosts.size();
    int totalPages = (totalPosts + POSTS_PER_PAGE - 1) / POSTS_PER_PAGE;

    for (int pageNum = 1; pageNum <= totalPages; pageNum++) {
        std::string blogListingHTML = generateBlogListingHTML(blogPosts, pageNum, POSTS_PER_PAGE);
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
    }

    // Generate HTML files only for pages that need updating
    for (const auto& page : pagesToGenerate) {
        std::string finalHtml = applyTemplate(templateContent, page.title, page.content, pages);
        std::string outputPath = outputDir + "/" + page.outputPath;
        writeFile(outputPath, finalHtml);
    }

    // Generate HTML files only for blog posts that need updating
    for (const auto& post : blogsToGenerate) {
        std::string finalHtml = applyTemplate(templateContent, post.title, post.content, pages, true);
        std::string outputPath = blogOutputDir + "/" + post.outputPath;
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
