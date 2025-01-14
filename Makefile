# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -I$(SRC_DIR) -pthread
LDFLAGS = -pthread

# Directories
SRC_DIR = src
BUILD_DIR = build
INCLUDE_DIR = include
RESOURCES_DIR = resources/database

# Source files
CORE_SRCS = $(SRC_DIR)/Core/Books.cpp $(SRC_DIR)/Core/Categories.cpp $(SRC_DIR)/Core/Users.cpp $(SRC_DIR)/Core/Transactions.cpp $(SRC_DIR)/Core/Audits.cpp
LIBRARY_SRCS = $(SRC_DIR)/Apis/Library.cpp
MANAGER_SRCS = $(SRC_DIR)/Apis/LibraryManager.cpp
SERVER_SRCS = $(SRC_DIR)/Network/LibraryServer.cpp
CLIENT_SRCS = $(SRC_DIR)/Network/LibraryClient.cpp

# Output executable
LIBRARY_EXE = $(BUILD_DIR)/library
#MANAGER_EXE = $(BUILD_DIR)/librarymanager

# Default target
all: setup $(LIBRARY_EXE) #$(MANAGER_EXE)

setup:
	mkdir -p $(BUILD_DIR)
	mkdir -p $(RESOURCES_DIR)
	mkdir -p $(INCLUDE_DIR)/nlohmann
	wget -O $(INCLUDE_DIR)/nlohmann/json.hpp https://github.com/nlohmann/json/releases/download/v3.11.2/json.hpp

$(LIBRARY_EXE): $(LIBRARY_SRCS) $(CORE_SRCS) $(SERVER_SRCS) $(CLIENT_SRCS) $(MANAGER_SRCS)
	$(CXX) $(CXXFLAGS) -I$(INCLUDE_DIR) $^ -o $@ $(LDFLAGS)
# $(MANAGER_EXE): $(MANAGER_SRCS) $(CORE_SRCS)
# 	$(CXX) $(CXXFLAGS) -I$(INCLUDE_DIR) $^ -o $@

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all setup clean