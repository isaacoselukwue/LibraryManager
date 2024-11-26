# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -I$(SRC_DIR)

# Directories
SRC_DIR = src
BUILD_DIR = build
INCLUDE_DIR = include
RESOURCES_DIR = resources/database

# Source files
CORE_SRCS = $(SRC_DIR)/Core/Books.cpp $(SRC_DIR)/Core/Categories.cpp $(SRC_DIR)/Core/Users.cpp
LIBRARY_SRCS = $(SRC_DIR)/Apis/Library.cpp

# Output executable
LIBRARY_EXE = $(BUILD_DIR)/library

# Default target
all: setup $(LIBRARY_EXE)

setup:
	mkdir -p $(BUILD_DIR)
	mkdir -p $(RESOURCES_DIR)
	mkdir -p $(INCLUDE_DIR)/nlohmann
	wget -O $(INCLUDE_DIR)/nlohmann/json.hpp https://github.com/nlohmann/json/releases/download/v3.11.2/json.hpp

$(LIBRARY_EXE): $(LIBRARY_SRCS) $(CORE_SRCS)
	$(CXX) $(CXXFLAGS) -I$(INCLUDE_DIR) $^ -o $@

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all setup clean