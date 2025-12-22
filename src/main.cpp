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
    std::string category;  // tech, movies, or random
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

std::string generateBlogListingHTML(const std::vector<BlogPost>& blogPosts, int pageNum, int postsPerPage, const std::string& category = "") {
    std::stringstream html;

    // Calculate pagination
    int totalPosts = blogPosts.size();
    int totalPages = (totalPosts + postsPerPage - 1) / postsPerPage;
    int startIdx = (pageNum - 1) * postsPerPage;
    int endIdx = std::min(startIdx + postsPerPage, totalPosts);

    // Category-specific titles
    std::string categoryTitle = category.empty() ? "All Posts" :
                                (category == "tech" ? "Tech Blog" :
                                 category == "movies" ? "Movie Reviews" : "Random Thoughts");

    std::string categoryDesc = category.empty() ? "All my blog posts" :
                               (category == "tech" ? "Technology, programming, and software development" :
                                category == "movies" ? "Movie reviews, analysis, and recommendations" :
                                "Random musings and miscellaneous topics");

    // Start blog container with sidebar
    html << "<div class=\"blog-container\">\n";

    // Sidebar
    html << "    <aside class=\"blog-sidebar\">\n";
    html << "        <nav class=\"sidebar-menu\">\n";
    html << "            <h3>Categories</h3>\n";
    html << "            <ul>\n";
    html << "                <li><a href=\"blogs.html\"" << (category.empty() ? " class=\"active\"" : "") << ">All Posts</a></li>\n";
    html << "                <li><a href=\"tech.html\"" << (category == "tech" ? " class=\"active\"" : "") << ">Tech</a></li>\n";
    html << "                <li><a href=\"movies.html\"" << (category == "movies" ? " class=\"active\"" : "") << ">Movies</a></li>\n";
    html << "                <li><a href=\"random.html\"" << (category == "random" ? " class=\"active\"" : "") << ">Thoughtful</a></li>\n";
    html << "            </ul>\n";
    html << "        </nav>\n";
    html << "    </aside>\n";

    // Main content
    html << "    <div class=\"blog-content\">\n";
    html << "        <h1>" << categoryTitle << "</h1>\n";
    html << "        <p>" << categoryDesc << "</p>\n\n";
    html << "        <div class=\"blog-list\">\n";

        // Show posts for current page
        for (int i = startIdx; i < endIdx; i++) {
            const auto& post = blogPosts[i];
            html << "            <article class=\"blog-item\">\n";

            // Add category badge if showing all posts (no category filter)
            if (category.empty() && !post.category.empty()) {
                std::string categoryDisplay = post.category;
                categoryDisplay[0] = std::toupper(categoryDisplay[0]); // Capitalize first letter
                html << "                <span class=\"category-badge category-" << post.category << "\">" << categoryDisplay << "</span>\n";
            }

            // Determine correct path - if it has a category, use blog/category/file.html, otherwise blog/file.html
            std::string postPath = post.category.empty() ? "blog/" + post.outputPath : "blog/" + post.category + "/" + post.outputPath;

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
    html << "</div>\n"; // close blog-container

    return html.str();
}

// Helper function to process blog posts from a specific category directory
void processCategoryBlogs(const std::string& categoryDir, const std::string& categoryOutputDir,
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
    std::string techDir = "content/blog/tech";
    std::string moviesDir = "content/blog/movies";
    std::string randomDir = "content/blog/random";
    std::string imagesDir = "content/images";
    std::string outputDir = "docs";
    std::string blogOutputDir = "docs/blog";
    std::string techOutputDir = "docs/blog/tech";
    std::string moviesOutputDir = "docs/blog/movies";
    std::string randomOutputDir = "docs/blog/random";
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
    if (!fs::exists(techOutputDir)) {
        fs::create_directory(techOutputDir);
    }
    if (!fs::exists(moviesOutputDir)) {
        fs::create_directory(moviesOutputDir);
    }
    if (!fs::exists(randomOutputDir)) {
        fs::create_directory(randomOutputDir);
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

    // Process tech blogs
    processCategoryBlogs(techDir, techOutputDir, "tech", blogPosts, blogsToGenerate,
                        skippedBlogs, cache, newCache, templateHash, parser);

    // Process movies blogs
    processCategoryBlogs(moviesDir, moviesOutputDir, "movies", blogPosts, blogsToGenerate,
                        skippedBlogs, cache, newCache, templateHash, parser);

    // Process random blogs
    processCategoryBlogs(randomDir, randomOutputDir, "random", blogPosts, blogsToGenerate,
                        skippedBlogs, cache, newCache, templateHash, parser);

    // Process uncategorized blogs (directly in content/blog)
    if (fs::exists(blogDir)) {
        for (const auto& entry : fs::directory_iterator(blogDir)) {
            // Skip directories (we already processed them above)
            if (entry.is_directory()) {
                continue;
            }

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
                std::string excerptMarkdown = extractExcerpt(markdownContent);
                std::string excerpt = parser.convertToHTML(excerptMarkdown);

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

                if (!needsBlogRegeneration(filepath, markdownContent, outputPath, templateHash, cache)) {
                    std::cout << "Skipping (up-to-date): " << filename << " [uncategorized]" << std::endl;
                    skippedBlogs++;
                    newCache[filepath] = cache[filepath];
                } else {
                    std::cout << "Processing blog: " << filename << " [uncategorized]" << std::endl;
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

                blogPosts.push_back(post);
            }
        }
    }

    // Sort blog posts by date (newest first)
    std::sort(blogPosts.begin(), blogPosts.end(), [](const BlogPost& a, const BlogPost& b) {
        return a.timestamp > b.timestamp;
    });

    const int POSTS_PER_PAGE = 5;

    // Generate main blog listing page (all posts from all categories)
    if (!blogPosts.empty()) {
        int totalPosts = blogPosts.size();
        int totalPages = (totalPosts + POSTS_PER_PAGE - 1) / POSTS_PER_PAGE;

        for (int pageNum = 1; pageNum <= totalPages; pageNum++) {
            std::string blogListingHTML = generateBlogListingHTML(blogPosts, pageNum, POSTS_PER_PAGE, "");
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
    std::vector<std::string> categories = {"tech", "movies", "random"};

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
            std::string categoryListingHTML = generateBlogListingHTML(categoryPosts, pageNum, POSTS_PER_PAGE, category);
            Page categoryIndexPage;
            categoryIndexPage.filename = category + ".md";
            categoryIndexPage.title = (category == "tech" ? "Tech Blog" :
                                      category == "movies" ? "Movie Reviews" : "Random Thoughts");
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
        if (post.category == "tech") {
            markdownPath = techDir + "/" + post.filename;
        } else if (post.category == "movies") {
            markdownPath = moviesDir + "/" + post.filename;
        } else if (post.category == "random") {
            markdownPath = randomDir + "/" + post.filename;
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
        if (post.category == "tech") {
            outputPath = techOutputDir + "/" + post.outputPath;
        } else if (post.category == "movies") {
            outputPath = moviesOutputDir + "/" + post.outputPath;
        } else if (post.category == "random") {
            outputPath = randomOutputDir + "/" + post.outputPath;
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
