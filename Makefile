# Simple Makefile for building the static site generator

CXX = g++
CXXFLAGS = -std=c++17 -Wall -Iinclude
LDFLAGS = -lsqlite3

# Executable name
TARGET = site_generator

# Source files
SOURCES = src/main.cpp src/markdown_parser.cpp src/blog_database.cpp src/jupyter_parser.cpp src/file_utils.cpp src/metadata.cpp src/cache.cpp src/toc.cpp src/category.cpp src/content_tree.cpp src/navigation.cpp src/sidebar.cpp src/template.cpp src/listing.cpp src/processor.cpp

# Object files
OBJECTS = $(SOURCES:.cpp=.o)

# Default target
all: $(TARGET)

# Build executable
$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJECTS) $(LDFLAGS)

# Compile source files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Run the generator
run: $(TARGET)
	./$(TARGET)

# Clean build files
clean:
	rm -f $(OBJECTS) $(TARGET)
	rm -f src/*.o

# Clean output
clean-output:
	rm -rf output/*

# Full rebuild
rebuild: clean all

.PHONY: all run clean clean-output rebuild
