# Deployment Guide - GitHub Pages

This guide will help you deploy your portfolio site to GitHub Pages with automatic rebuilds.

## Prerequisites

- Git installed
- GitHub account
- C++ compiler (for local testing)

## Quick Start - Deploy to GitHub

### 1. Create a GitHub Repository

```bash
# Initialize git repository (if not already done)
cd custom_blog_static_site_generator
git init

# Add all files
git add .

# Create initial commit
git commit -m "Initial commit: Portfolio site generator"
```

### 2. Push to GitHub

```bash
# Create a new repository on GitHub (via web interface)
# Then link your local repo to GitHub:

git remote add origin https://github.com/YOUR_USERNAME/YOUR_REPO_NAME.git
git branch -M main
git push -u origin main
```

### 3. Enable GitHub Pages

1. Go to your repository on GitHub
2. Click **Settings** → **Pages**
3. Under "Source", select:
   - **Source**: Deploy from a branch
   - **Branch**: `gh-pages`
   - **Folder**: `/ (root)`
4. Click **Save**

### 4. Wait for Deployment

- GitHub Actions will automatically build your site
- Check the **Actions** tab to see the build progress
- Once complete, your site will be live at:
  `https://YOUR_USERNAME.github.io/YOUR_REPO_NAME/`

## How It Works

### Automatic Deployment

Every time you push to the `main` branch:

1. GitHub Actions triggers (see `.github/workflows/build-site.yml`)
2. Compiles the C++ site generator
3. Runs the generator to create HTML files
4. Deploys the `docs/` folder to `gh-pages` branch
5. GitHub Pages serves your site

### Manual Deployment

If you prefer manual control:

```bash
# Build locally
make && ./site_generator

# Commit changes
git add content/ docs/
git commit -m "Update blog post"

# Push to trigger deployment
git push
```

Or use the deploy script:

```bash
# Linux/Mac
./scripts/deploy.sh

# Windows
scripts\deploy.bat
```

## Content Management

### Method 1: Web-based CMS (Easiest)

1. Open `cms/index.html` in your browser
2. Fill in the blog post form
3. Click "Download Markdown File"
4. Save the file to the `content/` directory
5. Commit and push:
   ```bash
   git add content/your-new-post.md
   git commit -m "Add new blog post"
   git push
   ```

### Method 2: Command-line Scripts

**Linux/Mac:**
```bash
./scripts/new-post.sh
# Follow the prompts
```

**Windows:**
```bash
scripts\new-post.bat
# Follow the prompts
```

### Method 3: Manual Creation

1. Create a new `.md` file in `content/`
2. Write your content in Markdown
3. Commit and push

## Updating Your Site

### Add a New Blog Post

```bash
# 1. Create the post (use CMS or scripts)
# 2. Test locally (optional)
make && ./site_generator
# Open docs/your-post.html in browser

# 3. Deploy
git add content/
git commit -m "Add blog post: Your Title"
git push
```

### Update Existing Content

```bash
# 1. Edit the .md file in content/
# 2. Commit and push
git add content/
git commit -m "Update about page"
git push
```

### Change Site Design

```bash
# 1. Edit templates/template.html
# 2. Rebuild and test locally
make && ./site_generator

# 3. Deploy
git add templates/
git commit -m "Update site design"
git push
```

## Troubleshooting

### Build Fails on GitHub Actions

Check the Actions tab for error logs. Common issues:

1. **Compilation errors**: Make sure code compiles locally first
2. **Missing files**: Ensure all files are committed
3. **Permissions**: Check workflow permissions in Settings → Actions

### Site Not Updating

1. Check Actions tab - build might have failed
2. Clear browser cache
3. Wait a few minutes for GitHub Pages to update
4. Verify `gh-pages` branch has new content

### Custom Domain

To use a custom domain:

1. Create a file `docs/CNAME` with your domain:
   ```
   yourdomain.com
   ```
2. Configure DNS with your domain provider:
   ```
   A Record: 185.199.108.153
   A Record: 185.199.109.153
   A Record: 185.199.110.153
   A Record: 185.199.111.153
   ```
3. Update GitHub Pages settings with your custom domain

## Project Structure for GitHub

```
your-repo/
├── .github/
│   └── workflows/
│       └── build-site.yml      # Auto-deployment workflow
├── cms/
│   └── index.html              # Web-based CMS
├── content/
│   ├── index.md                # Your content (commit these)
│   ├── about.md
│   └── blog-post.md
├── docs/                       # Generated HTML (auto-deployed)
│   ├── index.html
│   ├── about.html
│   └── blog-post.html
├── src/                        # C++ source code
├── templates/                  # HTML templates
└── scripts/                    # Helper scripts
```

## Tips

1. **Always commit `content/` and `docs/`** - Both are needed for deployment
2. **Test locally first** - Build and check before pushing
3. **Use meaningful commit messages** - Helps track changes
4. **Keep content separate** - Don't mix content and code changes
5. **Backup your content** - Git history serves as backup

## Example Workflow

```bash
# Morning: Write a blog post
open cms/index.html              # Use CMS to create post
# Save to content/my-post.md

# Build and preview
make && ./site_generator
open docs/my-post.html          # Check it looks good

# Deploy
git add content/my-post.md docs/
git commit -m "Add blog post: My Amazing Journey"
git push

# Wait 1-2 minutes, then check:
# https://YOUR_USERNAME.github.io/YOUR_REPO_NAME/my-post.html
```

## Need Help?

- Check GitHub Actions logs for build errors
- Review `.github/workflows/build-site.yml` for workflow details
- Ensure C++ code compiles locally before pushing
- Verify all paths use forward slashes (even on Windows)

---

**Your site will be live at:**
`https://YOUR_USERNAME.github.io/YOUR_REPO_NAME/`

Happy blogging!
