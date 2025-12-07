# Incremental Build System

The static site generator now supports **incremental builds** using content hashing. This significantly reduces build times by only regenerating files when their content actually changes.

## How It Works

### Two Detection Strategies

The system uses **different change detection** for pages vs blog posts:

**Regular Pages** (index.md, about.md, projects.md):
- Uses **file modification time** for change detection
- If you edit a page, the modification time changes → regenerates
- Works perfectly for frequently updated pages
- Fast comparison (no content hashing needed)

**Blog Posts** (content/blog/*.md):
- Uses **content hashing** for change detection
- Preserves original publish dates even when file timestamps change
- Works consistently across git checkouts and deployments
- Essential for keeping blog dates stable on GitHub Actions

### Cache File Format

The `.build_cache` file stores:
```
filepath contentHash timestamp fileModTime publishDate
```

Example:
```
content/blog/my-post.md 12345678 1638360000 0 December 01, 2021 at 10:30 AM
content/index.md 0 0 1709856000
```

- **contentHash**: Hash of content+template (blogs only, 0 for pages)
- **timestamp**: Publish timestamp (blogs only, 0 for pages)
- **fileModTime**: File modification time (pages only, 0 for blogs)
- **publishDate**: Human-readable date (blogs only, empty for pages)

## Key Features

### ✅ Dual Detection Strategy
- **Pages**: Use file modification time (detects edits immediately)
- **Blogs**: Use content hashing (preserves publish dates)
- Best of both worlds for different use cases

### ✅ Preserves Blog Publish Dates
- **First build**: Uses file modification date as publish date
- **Subsequent builds**: Preserves the original publish date from cache
- **GitHub Actions**: Blog dates stay consistent across deployments
- **Content changes**: Regenerates but keeps original date

### ✅ Responsive to Page Edits
- Edit index.md → File timestamp changes → Regenerates immediately
- Edit about.md → File timestamp changes → Regenerates immediately
- No content hashing overhead for frequently updated pages

### ✅ Detects Template Changes (for blogs)
- If `templates/template.html` changes, all BLOG posts rebuild
- Pages always regenerate if their modification time changed
- Smart detection prevents stale content

### ✅ Smart Skipping
- Skips unchanged files based on appropriate detection method
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
