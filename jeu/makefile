CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -g

BUILD_DIR = build
BIN_DIR = bin

SRC = $(wildcard *.cc)
OBJ = $(SRC:%.cc=$(BUILD_DIR)/%.o)

EXEC = $(BIN_DIR)/main

all: $(BUILD_DIR) $(BIN_DIR) $(EXEC)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(EXEC): $(OBJ)
	$(CXX) $(OBJ) -o $(EXEC)

$(BUILD_DIR)/%.o: %.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)

.PHONY: all clean