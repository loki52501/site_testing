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

- **Template System**: Uses HTML templates with placeholders
- **Auto-generated Navigation**: Automatically creates nav links for all pages
- **Clean Design**: Minimalistic, centered layout with responsive design
- **Easy to Update**: Just edit markdown files and rebuild
- **GitHub Pages Ready**: Automatic deployment via GitHub Actions
- **Web-based CMS**: Simple interface for creating blog posts
- **Helper Scripts**: Command-line tools for managing content

## Project Structure

```
custom_blog_static_site_generator/
├── .github/
│   └── workflows/
│       └── build-site.yml      # GitHub Actions auto-deployment
├── cms/
│   └── index.html              # Web-based CMS interface
├── include/
│   └── markdown_parser.h       # Header file for markdown parser
├── src/
│   ├── main.cpp                # Main program
│   └── markdown_parser.cpp     # Markdown parser implementation
├── templates/
│   └── template.html           # HTML template
├── content/
│   ├── index.md                # Home page
│   ├── about.md                # About page
│   └── projects.md             # Projects page
├── docs/                       # Generated HTML files (GitHub Pages)
├── scripts/
│   ├── new-post.sh             # Create new post (Linux/Mac)
│   ├── new-post.bat            # Create new post (Windows)
│   └── deploy.sh               # Build and deploy script
├── CMakeLists.txt              # CMake build configuration
├── Makefile                    # Simple Makefile
├── README.md                   # This file
└── DEPLOYMENT.md               # GitHub Pages deployment guide
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

## Learning Notes (for C++ Beginners)

This project demonstrates several C++ concepts:
- **Object-Oriented Programming**: Classes and methods
- **File I/O**: Reading and writing files
- **String Processing**: Regex, string manipulation
- **STL Containers**: vectors, maps, strings
- **Filesystem Library**: Directory iteration (C++17)
- **Build Systems**: Makefiles and CMake

## Future Enhancements

Possible improvements you can add:
- [ ] Support for custom metadata (dates, authors)
- [ ] CSS/JS asset copying
- [ ] Blog post sorting by date
- [ ] Syntax highlighting for code blocks
- [ ] RSS feed generation
- [ ] Custom page templates
- [ ] Live reload server

## License

Free to use and modify for your own projects.

## Contributing

Feel free to fork and improve! This is a learning project, so enhancements are welcome.
