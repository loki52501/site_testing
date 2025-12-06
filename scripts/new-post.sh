#!/bin/bash

# Script to create a new blog post

echo "=== Create New Blog Post ==="
echo ""

# Get post title
read -p "Enter post title: " title

# Generate filename from title (lowercase, replace spaces with hyphens)
filename=$(echo "$title" | tr '[:upper:]' '[:lower:]' | tr ' ' '-' | sed 's/[^a-z0-9-]//g')
filepath="content/${filename}.md"

# Check if file exists
if [ -f "$filepath" ]; then
    read -p "File already exists. Overwrite? (y/n): " overwrite
    if [ "$overwrite" != "y" ]; then
        echo "Cancelled."
        exit 0
    fi
fi

# Get current date
date=$(date +%Y-%m-%d)

# Create markdown file
cat > "$filepath" << EOF
# $title

*Published on $date*

Write your blog post content here...

## Section 1

Your content...

## Section 2

More content...

EOF

echo ""
echo "Created: $filepath"
echo ""
echo "Next steps:"
echo "1. Edit the file: $filepath"
echo "2. Build the site: make && ./site_generator"
echo "3. Commit and push to GitHub"
echo ""
