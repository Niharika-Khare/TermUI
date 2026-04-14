# Directory Paths for Project
BIN_DIR := ./bin
BUILD_DIR := ./build
INC_DIR :=./include
LIB_DIR := ./lib
SRC_DIR := ./src
TEST_DIR := ./tests
UNITY_DIR := ./unity

# Header files
INC_DIRS := $(shell find $(INC_DIR) -type d)
INC_FLAG := $(addprefix -I,$(INC_DIRS)) -I$(UNITY_DIR)

# Compiler settings
CC := cc
CFLAGS := $(INC_FLAG) -g

# Source files and Dependencies
DEPS := $(shell find $(INC_DIR) -name '*.h')
SRC := $(shell find $(SRC_DIR) -name '*.c')
OBJ := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRC))
TESTS := $(shell find $(TEST_DIR) -name '*.c')

#Targets
.PHONY: all test build clean help

all: test termui 
	$(BIN_DIR)/test
	$(BIN_DIR)/termui

build: $(BIN_DIR) $(BUILD_DIR) termui test

clean:
	rm -rf $(OBJ) $(BIN_DIR)/*

help:
	@echo "Makefile targets:"
	@echo "  all     - Build and run the tests and project (default)"
	@echo "  run     - Build the project but don't run"
	@echo "  clean   - Remove build artifacts"
	@echo "  test    - Run project tests"
	@echo "  help    - Show this message"

# Directory creation
$(BIN_DIR) $(BUILD_DIR):
	mkdir -p $@

# Object file creation
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c $(DEPS)
	@mkdir -p $(dir $@)
	$(CC) -c $< -o $@ $(CFLAGS) 

# Executable creation
termui: $(OBJ)
	$(CC) $^ -o $(BIN_DIR)/$@

# Unity compilation
$(BUILD_DIR)/unity.o: $(UNITY_DIR)/unity.c $(UNITY_DIR)/unity.h $(UNITY_DIR)/unity_internals.h
	$(CC) -c $< -o $@ -I$(UNITY_DIR)

# Test compilation
test: $(TESTS) $(BUILD_DIR)/unity.o 
	$(CC) $^ -o $(BIN_DIR)/$@ $(CFLAGS) 
	$(BIN_DIR)/$@