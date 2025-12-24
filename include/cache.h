#ifndef CACHE_H
#define CACHE_H

#include "structures.h"
#include <string>
#include <map>

// Simple hash function for content
size_t hashString(const std::string& str);

// Load cache from file
std::map<std::string, CachedMetadata> loadCache(const std::string& cacheFile);

// Save cache to file
void saveCache(const std::string& cacheFile, const std::map<std::string, CachedMetadata>& cache);

// Check if page needs regeneration based on file modification time
bool needsPageRegeneration(const std::string& sourcePath, const std::string& outputPath,
                           const std::map<std::string, CachedMetadata>& cache);

// Check if blog post needs regeneration based on content hash
bool needsBlogRegeneration(const std::string& sourcePath, const std::string& sourceContent,
                           const std::string& outputPath, const std::string& templateHash,
                           const std::map<std::string, CachedMetadata>& cache);

#endif // CACHE_H
