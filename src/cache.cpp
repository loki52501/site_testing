#include "cache.h"
#include "metadata.h"
#include <fstream>
#include <sstream>
#include <functional>
#include <filesystem>

namespace fs = std::filesystem;

size_t hashString(const std::string& str) {
    return std::hash<std::string>{}(str);
}

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
