# Directory Paths for Project
INC_DIR :=./include
SRC_DIR := ./src
LIB_DIR := ./lib
BUILD_DIR := ./build
TEST_DIR := ./tests
BIN_DIR := ./bin

# Compiler settings
CC := gcc
CFLAGS := -I$(INC_DIR)

# Source files and Dependencies
DEPS := $(wildcard $(INC_DIR)/*.h $(INC_DIR)/*/*.h)
SRC := $(wildcard $(SRC_DIR)/*.c $(SRC_DIR)/*/*.c)
OBJ := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRC))

#Targets
.PHONY: all build clean help

all: termui
	$(BIN_DIR)/termui

build: $(BIN_DIR) $(BUILD_DIR) termui 

clean:
	rm -rf $(OBJ) $(BIN_DIR)/*

help:
	@echo "Makefile targets:"
	@echo "  all     - Build and run the project (default)"
	@echo "  run     - Build the project but don't run"
	@echo "  clean   - Remove build artifacts"
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