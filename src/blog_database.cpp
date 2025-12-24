#include "../include/blog_database.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>

BlogDatabase::BlogDatabase(const std::string& dbPath) : db(nullptr), dbPath(dbPath) {}

BlogDatabase::~BlogDatabase() {
    if (db) {
        sqlite3_close(db);
    }
}

bool BlogDatabase::initialize() {
    // Delete existing database to start fresh
    std::remove(dbPath.c_str());

    int rc = sqlite3_open(dbPath.c_str(), &db);
    if (rc != SQLITE_OK) {
        std::cerr << "Cannot open database: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    const char* createTableSQL = R"(
        CREATE TABLE IF NOT EXISTS blogs (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            title TEXT NOT NULL,
            excerpt TEXT,
            category TEXT NOT NULL,
            publishDate TEXT NOT NULL,
            url TEXT NOT NULL,
            timestamp INTEGER NOT NULL
        );
        CREATE INDEX IF NOT EXISTS idx_category ON blogs(category);
        CREATE INDEX IF NOT EXISTS idx_timestamp ON blogs(timestamp DESC);
    )";

    char* errMsg = nullptr;
    rc = sqlite3_exec(db, createTableSQL, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false;
    }

    return true;
}

bool BlogDatabase::insertBlog(const BlogEntry& blog) {
    const char* insertSQL = R"(
        INSERT INTO blogs (title, excerpt, category, publishDate, url, timestamp)
        VALUES (?, ?, ?, ?, ?, ?);
    )";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, insertSQL, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    sqlite3_bind_text(stmt, 1, blog.title.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, blog.excerpt.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, blog.category.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, blog.publishDate.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, blog.url.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int64(stmt, 6, static_cast<sqlite3_int64>(blog.timestamp));

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        std::cerr << "Failed to insert blog: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    return true;
}

std::vector<BlogEntry> BlogDatabase::getAllBlogs() {
    std::vector<BlogEntry> blogs;

    const char* selectSQL = "SELECT id, title, excerpt, category, publishDate, url, timestamp FROM blogs ORDER BY timestamp DESC;";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, selectSQL, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare SELECT: " << sqlite3_errmsg(db) << std::endl;
        return blogs;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        BlogEntry blog;
        blog.id = sqlite3_column_int(stmt, 0);
        blog.title = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));

        const unsigned char* excerptText = sqlite3_column_text(stmt, 2);
        blog.excerpt = excerptText ? reinterpret_cast<const char*>(excerptText) : "";

        blog.category = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        blog.publishDate = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        blog.url = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        blog.timestamp = sqlite3_column_int64(stmt, 6);

        blogs.push_back(blog);
    }

    sqlite3_finalize(stmt);
    return blogs;
}

std::string BlogDatabase::escapeJSON(const std::string& str) {
    std::string escaped;
    escaped.reserve(str.length());

    for (char c : str) {
        switch (c) {
            case '"':  escaped += "\\\""; break;
            case '\\': escaped += "\\\\"; break;
            case '\b': escaped += "\\b"; break;
            case '\f': escaped += "\\f"; break;
            case '\n': escaped += "\\n"; break;
            case '\r': escaped += "\\r"; break;
            case '\t': escaped += "\\t"; break;
            default:
                if (c >= 0 && c < 32) {
                    // Control characters
                    char buf[7];
                    snprintf(buf, sizeof(buf), "\\u%04x", c);
                    escaped += buf;
                } else {
                    escaped += c;
                }
        }
    }

    return escaped;
}

bool BlogDatabase::exportToJSON(const std::string& jsonPath) {
    std::vector<BlogEntry> blogs = getAllBlogs();

    std::ofstream file(jsonPath);
    if (!file.is_open()) {
        std::cerr << "Cannot open JSON file for writing: " << jsonPath << std::endl;
        return false;
    }

    file << "{\n";
    file << "  \"blogs\": [\n";

    for (size_t i = 0; i < blogs.size(); ++i) {
        const BlogEntry& blog = blogs[i];

        file << "    {\n";
        file << "      \"id\": " << blog.id << ",\n";
        file << "      \"title\": \"" << escapeJSON(blog.title) << "\",\n";
        file << "      \"excerpt\": \"" << escapeJSON(blog.excerpt) << "\",\n";
        file << "      \"category\": \"" << escapeJSON(blog.category) << "\",\n";
        file << "      \"publishDate\": \"" << escapeJSON(blog.publishDate) << "\",\n";
        file << "      \"url\": \"" << escapeJSON(blog.url) << "\",\n";
        file << "      \"timestamp\": " << blog.timestamp << "\n";
        file << "    }";

        if (i < blogs.size() - 1) {
            file << ",";
        }
        file << "\n";
    }

    file << "  ]\n";
    file << "}\n";

    file.close();
    std::cout << "Exported " << blogs.size() << " blogs to " << jsonPath << std::endl;
    return true;
}

void BlogDatabase::clearBlogs() {
    const char* deleteSQL = "DELETE FROM blogs;";
    char* errMsg = nullptr;
    int rc = sqlite3_exec(db, deleteSQL, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to clear blogs: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    }
}
