#ifndef BLOG_DATABASE_H
#define BLOG_DATABASE_H

#include <string>
#include <vector>
#include <ctime>
#include <sqlite3.h>

struct BlogEntry {
    int id;
    std::string title;
    std::string excerpt;
    std::string category;
    std::string publishDate;
    std::string url;
    std::time_t timestamp;
};

class BlogDatabase {
public:
    BlogDatabase(const std::string& dbPath);
    ~BlogDatabase();

    bool initialize();
    bool insertBlog(const BlogEntry& blog);
    std::vector<BlogEntry> getAllBlogs();
    bool exportToJSON(const std::string& jsonPath);
    void clearBlogs();

private:
    sqlite3* db;
    std::string dbPath;

    std::string escapeJSON(const std::string& str);
};

#endif // BLOG_DATABASE_H
