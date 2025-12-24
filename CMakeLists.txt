cmake_minimum_required(VERSION 3.10)
project(MarkdownSiteGenerator)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(include)

# Source files
set(SOURCES
    src/main.cpp
    src/markdown_parser.cpp
    src/blog_database.cpp
    src/jupyter_parser.cpp
    src/file_utils.cpp
    src/metadata.cpp
    src/cache.cpp
    src/toc.cpp
    src/category.cpp
    src/content_tree.cpp
    src/navigation.cpp
    src/sidebar.cpp
    src/template.cpp
    src/listing.cpp
    src/processor.cpp
)

# Create executable
add_executable(site_generator ${SOURCES})

# Link SQLite3
find_package(SQLite3 REQUIRED)
target_link_libraries(site_generator SQLite::SQLite3)

# For Windows, link against stdc++fs if needed
if(WIN32)
    # target_link_libraries(site_generator) # Already linked SQLite3 above
endif()

# For Linux/Mac, may need to link filesystem library
if(UNIX)
    target_link_libraries(site_generator stdc++fs)
endif()
