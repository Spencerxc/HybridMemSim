# Makefile for HybridMemSim

CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Iinclude
LDFLAGS =

SRC_DIR = src
INC_DIR = include
BUILD_DIR = build
BIN = $(BUILD_DIR)/HybridMemSim

# Source files
SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS = $(SOURCES:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)

# Default target
all: $(BUILD_DIR) $(BIN)

# Create build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Link
$(BIN): $(OBJECTS)
	$(CXX) $(LDFLAGS) $^ -o $@

# Compile (ensure build dir exists before compiling objects)
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean
clean:
	rm -rf $(BUILD_DIR)/*

# Run
run: $(BIN)
	./$(BIN) ./config/default.cfg

.PHONY: all clean run
