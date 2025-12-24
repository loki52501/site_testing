# Search Blogs

<div class="search-container">
    <input type="text" id="blog-search" placeholder="Search blog posts..." aria-label="Search blogs">

    <div class="search-controls">
        <div class="category-filters">
            <span style="font-size: 0.875rem; color: #718096; margin-right: 1rem;">Filter by:</span>
            <a href="#" class="category-filter" data-category="tech">Tech</a>
            <a href="#" class="category-filter" data-category="movies">Movies</a>
            <a href="#" class="category-filter" data-category="random">Random</a>
        </div>

        <div class="sort-control">
            <label for="blog-sort">Sort by:</label>
            <select id="blog-sort">
                <option value="date-desc">Newest First</option>
                <option value="date-asc">Oldest First</option>
                <option value="title-asc">Title (A-Z)</option>
                <option value="title-desc">Title (Z-A)</option>
            </select>
        </div>
    </div>

    <p id="result-count" class="result-count"></p>
</div>

<div id="search-results" class="blog-list"></div>

<script src="search.js"></script>
