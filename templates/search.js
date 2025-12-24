// Blog Search and Filter Functionality
class BlogSearch {
    constructor() {
        this.blogs = [];
        this.filteredBlogs = [];
        this.currentCategory = '';
        this.currentSort = 'date-desc';
        this.searchTerm = '';
        this.isSearching = false;
        this.originalContent = '';

        this.init();
    }

    async init() {
        try {
            // Try to use embedded data first (avoids CORS issues with file:// protocol)
            console.log('BlogSearch initializing...');
            console.log('window.BLOG_DATA exists:', !!window.BLOG_DATA);

            if (window.BLOG_DATA && window.BLOG_DATA.blogs) {
                this.blogs = window.BLOG_DATA.blogs;
                console.log('Using embedded blog data, loaded', this.blogs.length, 'blogs');
            } else {
                // Fallback to fetching if embedded data not available
                console.log('Embedded data not available, attempting fetch...');
                const response = await fetch('blogs.json');
                const data = await response.json();
                this.blogs = data.blogs;
                console.log('Fetched blog data, loaded', this.blogs.length, 'blogs');
            }

            this.filteredBlogs = [...this.blogs];

            // Save original content for restore
            const container = document.getElementById('search-results');
            if (container) {
                this.originalContent = container.innerHTML;
                console.log('Saved original content');
            } else {
                console.warn('Search results container not found');
            }

            this.setupEventListeners();
            console.log('Event listeners set up');
            this.updateResultCount();
            console.log('BlogSearch initialization complete');
        } catch (error) {
            console.error('Failed to load blog data:', error);
        }
    }

    setupEventListeners() {
        const searchInput = document.getElementById('blog-search');
        const categoryFilters = document.querySelectorAll('.category-filter');
        const sortSelect = document.getElementById('blog-sort');

        if (searchInput) {
            searchInput.addEventListener('input', (e) => {
                this.searchTerm = e.target.value.toLowerCase();
                this.filterAndSort();
            });
        }

        if (categoryFilters) {
            categoryFilters.forEach(filter => {
                filter.addEventListener('click', (e) => {
                    e.preventDefault();
                    const category = e.target.dataset.category;
                    this.currentCategory = this.currentCategory === category ? '' : category;

                    // Update active state
                    categoryFilters.forEach(f => f.classList.remove('active'));
                    if (this.currentCategory) {
                        e.target.classList.add('active');
                    }

                    this.filterAndSort();
                });
            });
        }

        if (sortSelect) {
            sortSelect.addEventListener('change', (e) => {
                this.currentSort = e.target.value;
                this.filterAndSort();
            });
        }
    }

    filterAndSort() {
        // Check if user is actively searching/filtering
        this.isSearching = this.searchTerm !== '' || this.currentCategory !== '';

        if (!this.isSearching) {
            // Restore original content if not searching
            const container = document.getElementById('search-results');
            if (container && this.originalContent) {
                container.innerHTML = this.originalContent;
            }
            // Show pagination
            const pagination = document.querySelector('.pagination');
            if (pagination) pagination.style.display = 'flex';
            this.updateResultCount();
            return;
        }

        // Hide pagination when searching
        const pagination = document.querySelector('.pagination');
        if (pagination) pagination.style.display = 'none';

        // Start with all blogs
        this.filteredBlogs = [...this.blogs];

        // Apply search filter
        if (this.searchTerm) {
            this.filteredBlogs = this.filteredBlogs.filter(blog => {
                const titleMatch = blog.title.toLowerCase().includes(this.searchTerm);
                const excerptMatch = blog.excerpt.toLowerCase().includes(this.searchTerm);
                return titleMatch || excerptMatch;
            });
        }

        // Apply category filter
        if (this.currentCategory) {
            this.filteredBlogs = this.filteredBlogs.filter(blog =>
                blog.category === this.currentCategory
            );
        }

        // Apply sorting
        switch (this.currentSort) {
            case 'date-desc':
                this.filteredBlogs.sort((a, b) => b.timestamp - a.timestamp);
                break;
            case 'date-asc':
                this.filteredBlogs.sort((a, b) => a.timestamp - b.timestamp);
                break;
            case 'title-asc':
                this.filteredBlogs.sort((a, b) => a.title.localeCompare(b.title));
                break;
            case 'title-desc':
                this.filteredBlogs.sort((a, b) => b.title.localeCompare(a.title));
                break;
        }

        this.renderResults();
    }

    updateResultCount() {
        const countElement = document.getElementById('result-count');
        if (!countElement) return;

        if (!this.isSearching) {
            countElement.textContent = '';
            return;
        }

        const total = this.blogs.length;
        const shown = this.filteredBlogs.length;
        countElement.textContent = shown === total
            ? `Showing all ${total} posts`
            : `Showing ${shown} of ${total} posts`;
    }

    renderResults() {
        const container = document.getElementById('search-results');
        if (!container) return;

        if (this.filteredBlogs.length === 0) {
            container.innerHTML = `
                <div style="text-align: center; padding: 4rem 2rem; color: #718096;">
                    <p style="font-size: 1.125rem;">No blogs found matching your criteria.</p>
                    <p style="font-size: 0.875rem; margin-top: 1rem;">Try adjusting your search or filters.</p>
                </div>
            `;
            return;
        }

        const resultsHTML = this.filteredBlogs.map(blog => {
            const categoryClass = `category-${blog.category}`;
            const categoryLabel = blog.category.charAt(0).toUpperCase() + blog.category.slice(1);

            return `
                <article class="blog-item">
                    <span class="category-badge ${categoryClass}">${categoryLabel}</span>
                    <h2><a href="${blog.url}">${this.escapeHTML(blog.title)}</a></h2>
                    <p class="blog-date">Published on ${blog.publishDate}</p>
                    ${blog.excerpt ? `<p class="blog-excerpt">${blog.excerpt}</p>` : ''}
                    <a href="${blog.url}" class="read-more">Read more →</a>
                </article>
            `;
        }).join('');

        container.innerHTML = resultsHTML;
        this.updateResultCount();
    }

    escapeHTML(text) {
        const div = document.createElement('div');
        div.textContent = text;
        return div.innerHTML;
    }
}

// Initialize search when DOM is loaded
if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', () => {
        new BlogSearch();
    });
} else {
    new BlogSearch();
}
