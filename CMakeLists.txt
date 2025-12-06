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
)

# Create executable
add_executable(site_generator ${SOURCES})

# For Windows, link against stdc++fs if needed
if(WIN32)
    target_link_libraries(site_generator)
endif()

# For Linux/Mac, may need to link filesystem library
if(UNIX)
    target_link_libraries(site_generator stdc++fs)
endif()
