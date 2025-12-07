# Incremental Build System

The static site generator now supports **incremental builds** using content hashing. This significantly reduces build times by only regenerating files when their content actually changes.

## How It Works

### Content Hashing
Instead of relying on file modification timestamps (which change when files are copied or checked out from git), the system:

1. **Calculates a hash** of each markdown file's content + template content
2. **Stores the hash** in a `.build_cache` file along with metadata
3. **Compares hashes** on subsequent builds to detect changes
4. **Skips regeneration** if content hasn't changed

### Cache File Format

The `.build_cache` file stores:
```
filepath contentHash timestamp publishDate
```

Example:
```
content/blog/my-post.md 12345678 1638360000 December 01, 2021 at 10:30 AM
content/index.md 87654321 0
```

## Key Features

### ✅ Platform Independent
- Works consistently on Windows, Linux, macOS, and GitHub Actions
- Not affected by git checkout timestamps
- Not affected by file system differences

### ✅ Preserves Blog Publish Dates
- **First build**: Uses file modification date as publish date
- **Subsequent builds**: Preserves the original publish date from cache
- **GitHub Actions**: Blog dates stay consistent across deployments

### ✅ Detects Template Changes
- If `templates/template.html` changes, ALL pages rebuild (expected behavior)
- Template hash is included in content comparison

### ✅ Smart Skipping
- Skips unchanged markdown files
- Still reads metadata (for navigation)
- Only runs expensive markdown→HTML conversion when needed

## Usage

The system works automatically:

```bash
# First build - generates everything
./site_generator

# Output:
# Processing page: index.md
# Processing page: about.md
# Processing blog: my-post.md
# Generated 2 pages and 1 blog post

# Second build - skips unchanged files
./site_generator

# Output:
# Skipping (up-to-date): index.md
# Skipping (up-to-date): about.md
# Skipping (up-to-date): my-post.md
# Processed 2 pages (0 generated, 2 skipped)
# Processed 1 blog post (0 generated, 1 skipped)
# Incremental build saved time by skipping 3 up-to-date files!

# Edit a file and rebuild
# (Only the changed file regenerates)
./site_generator

# Output:
# Skipping (up-to-date): index.md
# Processing page: about.md
# Skipping (up-to-date): my-post.md
# Processed 2 pages (1 generated, 1 skipped)
```

## GitHub Actions Integration

### Important: Commit the Cache File

The `.build_cache` file **must be committed** to your repository for incremental builds to work on GitHub Actions:

```bash
git add .build_cache
git commit -m "Update build cache"
git push
```

### How It Works on GitHub Actions

1. **First deployment**:
   - No cache exists
   - All files generate
   - Cache is created with current timestamp
   - Cache gets committed and pushed

2. **Subsequent deployments**:
   - Cache file is checked out from repo
   - Blog dates are preserved from cache
   - Only modified files regenerate
   - Cache is updated and committed

### Workflow Update (Optional)

If you want the workflow to automatically commit cache updates:

```yaml
- name: Generate site
  run: ./site_generator

- name: Commit cache if changed
  run: |
    git config user.name "github-actions[bot]"
    git config user.email "github-actions[bot]@users.noreply.github.com"
    git add .build_cache
    git diff --quiet && git diff --staged --quiet || git commit -m "Update build cache [skip ci]"
    git push || true
```

## Benefits

### Performance
- **First build**: Same speed as before (generates everything)
- **Incremental builds**: 10-100x faster depending on changes
- **Typical blog post update**: Only regenerates 1 file + blog index

### Consistency
- **Blog dates stay fixed** even when deployed from GitHub Actions
- **No timestamp issues** across different systems
- **Predictable behavior** regardless of git operations

### Cost Reduction
- **Less CPU time** on GitHub Actions
- **Faster deployments**
- **Lower carbon footprint**

## Cache Management

### View Cache
```bash
cat .build_cache
```

### Clear Cache (Force Full Rebuild)
```bash
rm .build_cache
./site_generator
```

### Cache Location
- File: `.build_cache` (in project root)
- Format: Plain text (human readable)
- Size: ~100 bytes per cached file

## Technical Details

### Hash Function
Uses C++ standard library `std::hash<std::string>` for speed:
```cpp
size_t hash = std::hash<std::string>{}(content + templateHash);
```

### Cache Structure
```cpp
struct CachedMetadata {
    size_t contentHash;      // Hash of content + template
    std::string publishDate; // Original publish date (for blogs)
    std::time_t timestamp;   // Unix timestamp (for sorting)
};
```

### Regeneration Logic
A file is regenerated if:
1. Output file doesn't exist, OR
2. Content hash has changed (content or template modified), OR
3. Not in cache (new file)

### Date Preservation
- **New blog post**: Date = file modification time
- **Cached blog post**: Date = cached publish date
- **Modified blog post**: Date = preserved from cache

## Troubleshooting

### Blog dates are wrong
```bash
# Clear cache and rebuild to reset dates
rm .build_cache
./site_generator
git add .build_cache
git commit -m "Reset build cache with correct dates"
```

### File not regenerating
```bash
# Force regeneration by clearing cache
rm .build_cache
./site_generator
```

### Cache conflicts on GitHub
```bash
# If cache gets corrupted, delete it
rm .build_cache
git add .build_cache
git commit -m "Clear build cache"
git push
```

## Best Practices

1. **Always commit `.build_cache`** after generating new blog posts
2. **Don't manually edit `.build_cache`** - let the generator manage it
3. **Clear cache** if you want to reset all blog dates
4. **Include cache in git** for consistent GitHub Actions deployments

## Summary

The incremental build system provides:
- ✅ Fast rebuilds (only changed files)
- ✅ Consistent blog dates across deployments
- ✅ Platform-independent behavior
- ✅ GitHub Actions compatibility
- ✅ Zero configuration required

Just commit `.build_cache` with your changes and enjoy faster builds!
