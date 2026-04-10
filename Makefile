# Directory Paths for Project
INC_DIR :=./include
SRC_DIR := ./src
LIB_DIR := ./lib
BUILD_DIR := ./build
TEST_DIR := ./tests
BIN_DIR := ./bin

# Header files
INC_DIRS := $(shell find $(INC_DIR) -type d)
INC_FLAG := $(addprefix -I,$(INC_DIRS))

# Compiler settings
CC := cc
CFLAGS := $(INC_FLAG)

# Source files and Dependencies
DEPS := $(shell find $(INC_DIR) -name '*.h')
SRC := $(shell find $(SRC_DIR) -name '*.c')
OBJ := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRC))

#Targets
.PHONY: all tests build clean help

all: termui
	$(BIN_DIR)/termui

tests: 
	@echo "Run tests" 

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