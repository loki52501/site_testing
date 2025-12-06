#!/bin/bash

# Script to build and deploy the site

echo "=== Building and Deploying Site ==="
echo ""

# Build the generator
echo "1. Building site generator..."
make clean
make

if [ $? -ne 0 ]; then
    echo "Error: Build failed!"
    exit 1
fi

# Generate the site
echo ""
echo "2. Generating site..."
./site_generator

if [ $? -ne 0 ]; then
    echo "Error: Site generation failed!"
    exit 1
fi

# Git operations
echo ""
echo "3. Preparing to commit..."
git add content/ docs/

echo ""
read -p "Enter commit message (or press Enter for default): " commit_msg

if [ -z "$commit_msg" ]; then
    commit_msg="Update site content"
fi

git commit -m "$commit_msg"

echo ""
read -p "Push to GitHub? (y/n): " push

if [ "$push" == "y" ]; then
    echo "Pushing to GitHub..."
    git push
    echo ""
    echo "Done! Your site will be deployed automatically via GitHub Actions."
    echo "Check: https://github.com/YOUR_USERNAME/YOUR_REPO/actions"
else
    echo "Not pushed. Run 'git push' when ready."
fi

echo ""
