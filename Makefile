CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O3 -Iinclude
SRC_DIR = src
INC_DIR = include
BIN_DIR = bin
TEST_DIR = tests
OBJ_DIR = obj

SRCS = $(SRC_DIR)/BitStream.cpp \
       $(SRC_DIR)/HuffmanTree.cpp \
       $(SRC_DIR)/FileUtils.cpp \
       $(SRC_DIR)/HuffmanCompressor.cpp

OBJS = $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRCS))
MAIN_OBJ = $(OBJ_DIR)/main.o
TEST_OBJ = $(OBJ_DIR)/test_runner.o

TARGET = $(BIN_DIR)/huffman
TEST_TARGET = $(BIN_DIR)/test_runner

.PHONY: all clean test run

all: $(TARGET)

$(TARGET): $(OBJS) $(MAIN_OBJ) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(TEST_TARGET): $(OBJS) $(TEST_OBJ) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/test_runner.o: $(TEST_DIR)/test_runner.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BIN_DIR) $(OBJ_DIR):
	mkdir -p $@

test: $(TEST_TARGET)
	@echo "Running tests..."
	@./$(TEST_TARGET)

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR) tests/temp_* examples/*.huf examples/*_decompressed.*


