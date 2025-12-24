# Custom Static Site Generator

A lightweight, custom-built static site generator written in C++ that converts Markdown files to HTML. Perfect for creating a personal portfolio website that's easy to update and deploy to GitHub Pages.

## Features

- **Markdown to HTML Conversion**: Supports all common markdown syntax
  - Headers (h1-h6)
  - Bold and italic text
  - Links and images
  - Lists (ordered and unordered)
  - Code blocks and inline code
  - Paragraphs

- **Jupyter Notebook Support**: Convert .ipynb files to HTML
- **Modular Architecture**: Clean, organized codebase split into feature-based modules
- **Dynamic Categories**: Auto-discover categories from folder structure
- **Incremental Builds**: Smart caching to only rebuild changed files
- **Hierarchical Content**: Support for nested subcategories
- **Search & Filter**: Client-side search with category filtering
- **SQLite Database**: Structured blog metadata storage
- **Table of Contents**: Auto-generated TOCs for long posts
- **Pagination**: Automatic pagination for blog listings
- **Template System**: Flexible HTML templates with placeholders
- **Auto-generated Navigation**: Dynamic nav links and sidebars
- **Clean Design**: Minimalistic, responsive layout
- **GitHub Pages Ready**: Automatic deployment via GitHub Actions
- **Web-based CMS**: Simple interface for creating blog posts

## Project Structure

```
custom_blog_static_site_generator/
├── .github/
│   └── workflows/
│       └── build-site.yml          # GitHub Actions auto-deployment
├── cms/
│   └── index.html                  # Web-based CMS interface
├── include/                        # Header files (modular architecture)
│   ├── structures.h                # Data structures (Page, BlogPost, etc.)
│   ├── file_utils.h                # File I/O operations
│   ├── metadata.h                  # Title/excerpt extraction
│   ├── cache.h                     # Build cache management
│   ├── toc.h                       # Table of contents generation
│   ├── category.h                  # Category management
│   ├── content_tree.h              # Content tree building
│   ├── navigation.h                # Navigation menu generation
│   ├── sidebar.h                   # Sidebar generation
│   ├── template.h                  # Template application
│   ├── listing.h                   # Blog listing pages
│   ├── processor.h                 # Content processing
│   ├── markdown_parser.h           # Markdown parser
│   ├── blog_database.h             # SQLite database
│   └── jupyter_parser.h            # Jupyter notebook support
├── src/                            # Implementation files
│   ├── main.cpp                    # Main orchestrator (simplified)
│   ├── file_utils.cpp              # File operations
│   ├── metadata.cpp                # Metadata extraction
│   ├── cache.cpp                   # Cache management
│   ├── toc.cpp                     # TOC generation
│   ├── category.cpp                # Category handling
│   ├── content_tree.cpp            # Tree building
│   ├── navigation.cpp              # Nav rendering
│   ├── sidebar.cpp                 # Sidebar rendering
│   ├── template.cpp                # Template processing
│   ├── listing.cpp                 # Listing generation
│   ├── processor.cpp               # Content processing
│   ├── markdown_parser.cpp         # Markdown parsing
│   ├── blog_database.cpp           # Database operations
│   └── jupyter_parser.cpp          # Notebook parsing
├── templates/
│   ├── template.html               # HTML template
│   ├── style.css                   # Stylesheet
│   └── search.js                   # Search functionality
├── content/
│   ├── blog/                       # Blog posts (organized by category)
│   │   ├── tech/                   # Tech category
│   │   ├── books/                  # Books category
│   │   ├── movies/                 # Movies category
│   │   └── random/                 # Random category
│   ├── index.md                    # Home page
│   ├── about.md                    # About page
│   └── projects.md                 # Projects page
├── docs/                           # Generated HTML (GitHub Pages)
├── scripts/
│   ├── new-post.sh                 # Create new post (Linux/Mac)
│   ├── new-post.bat                # Create new post (Windows)
│   └── deploy.sh                   # Build and deploy
├── CMakeLists.txt                  # CMake build configuration
├── Makefile                        # Make build configuration
├── README.md                       # This file
├── DEPLOYMENT.md                   # Deployment guide
├── MODULARIZATION_STATUS.md        # Module documentation
└── .build_cache                    # Incremental build cache
```

## Building the Project

### Option 1: Using Make (Recommended for quick builds)

```bash
# Build the project
make

# Build and run
make run

# Clean build files
make clean

# Full rebuild
make rebuild
```

### Option 2: Using CMake

```bash
# Create build directory
mkdir build
cd build

# Configure and build
cmake ..
cmake --build .

# Run the generator
./site_generator
```

### Windows (with MinGW or MSVC)

```bash
# Using g++ directly
g++ -std=c++17 -Iinclude src/main.cpp src/markdown_parser.cpp -o site_generator.exe

# Run
site_generator.exe
```

## Quick Start

### Local Development

1. **Build the site generator**
   ```bash
   make
   ```

2. **Create content** using one of these methods:

   **Option A: Web CMS (Easiest)**
   - Open `cms/index.html` in your browser
   - Fill in the form and download the `.md` file
   - Save it to the `content/` directory

   **Option B: Command-line**
   ```bash
   # Linux/Mac
   ./scripts/new-post.sh

   # Windows
   scripts\new-post.bat
   ```

   **Option C: Manual**
   - Create a `.md` file in `content/`
   - Write your content in Markdown

3. **Generate the site**
   ```bash
   ./site_generator
   ```

4. **View locally**
   ```bash
   # Open in browser
   open docs/index.html       # Mac
   start docs/index.html      # Windows
   xdg-open docs/index.html   # Linux
   ```

### Deploy to GitHub Pages

See [DEPLOYMENT.md](DEPLOYMENT.md) for detailed instructions.

**Quick version:**
```bash
# 1. Create GitHub repo and push
git init
git add .
git commit -m "Initial commit"
git remote add origin https://github.com/YOUR_USERNAME/YOUR_REPO.git
git push -u origin main

# 2. Enable GitHub Pages in repo settings
#    Settings → Pages → Source: gh-pages branch

# 3. Push updates
git add content/ docs/
git commit -m "Add blog post"
git push
# Site auto-deploys via GitHub Actions!
```

## Usage

### Creating Blog Posts

**Method 1: Web-based CMS** (Recommended)
1. Open `cms/index.html` in your browser
2. Fill in the form with your blog post
3. Click "Download Markdown File"
4. Save to `content/` directory
5. Commit and push to deploy

**Method 2: Command-line Scripts**
```bash
# Linux/Mac
./scripts/new-post.sh

# Windows
scripts\new-post.bat
```

**Method 3: Manual Creation**
Create a file `content/my-post.md`:
```markdown
# My Blog Post

This is a blog post with **bold** and *italic* text.

## Features
- Easy to write
- Clean output
```

## Customization

### Changing the Design

Edit the CSS in `templates/template.html`:
- Modify colors, fonts, spacing
- Adjust the max-width for content area
- Change navigation styling

### Adding New Pages

1. Create a new `.md` file in `content/`
2. Run the generator
3. The page will automatically appear in the navigation

### Markdown Syntax Examples

```markdown
# Heading 1
## Heading 2

**Bold text** and *italic text*

[Link text](https://example.com)

![Image alt text](image.jpg)

- List item 1
- List item 2

1. Numbered item
2. Another item

`inline code`

\```
code block
\```
```

## Requirements

- C++17 compatible compiler (GCC, Clang, MSVC)
- Standard library with filesystem support
- CMake 3.10+ (if using CMake)
- Make (if using Makefile)

## Architecture

This project follows a **modular architecture** with clear separation of concerns:

### Core Modules
- **structures**: Shared data structures used across modules
- **file_utils**: File reading/writing operations
- **metadata**: Extract titles, excerpts, and dates from content
- **cache**: Incremental build system using file hashes

### Content Management
- **content_tree**: Build hierarchical content structure from filesystem
- **category**: Dynamic category discovery and management
- **processor**: Process and convert content files to HTML

### HTML Generation
- **template**: Apply HTML templates with placeholder replacement
- **navigation**: Generate navigation menus
- **sidebar**: Generate category sidebars with nesting
- **toc**: Generate table of contents from headings
- **listing**: Generate paginated blog listing pages

### Existing Modules
- **markdown_parser**: Convert Markdown to HTML
- **jupyter_parser**: Convert Jupyter notebooks to HTML
- **blog_database**: SQLite database for blog metadata

This architecture makes it easy to:
- Add new features (e.g., RSS feed generation)
- Modify specific functionality without touching other code
- Test individual modules
- Understand code organization by feature

## Learning Notes (for C++ Beginners)

This project demonstrates several C++ concepts:
- **Modular Design**: Feature-based code organization
- **Object-Oriented Programming**: Classes and methods
- **File I/O**: Reading and writing files
- **String Processing**: Regex, string manipulation
- **STL Containers**: vectors, maps, sets, strings
- **Filesystem Library**: Directory iteration (C++17)
- **Build Systems**: Makefiles and CMake
- **Memory Management**: Manual tree allocation/deallocation

## Future Enhancements

Possible improvements you can add:
- [x] Support for custom metadata (dates, categories) ✅
- [x] CSS/JS asset copying ✅
- [x] Blog post sorting by date ✅
- [x] Incremental builds with caching ✅
- [x] Dynamic category discovery ✅
- [x] Hierarchical content structure ✅
- [x] Jupyter notebook support ✅
- [ ] RSS feed generation (planned - easy to add as new module!)
- [ ] Syntax highlighting for code blocks
- [ ] Custom page templates
- [ ] Live reload server
- [ ] Sitemap generation
- [ ] Social media meta tags

## License

Free to use and modify for your own projects.

## Contributing

Feel free to fork and improve! This is a learning project, so enhancements are welcome.
