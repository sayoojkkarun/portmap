# Makefile for portmap
# Learning C: A Makefile automates the build process

# Compiler and flags
CC = gcc
# Strict flags for automotive/MISRA C compliance
CFLAGS = -Wall -Wextra -Werror -Iinclude -std=c11 -pedantic
LDFLAGS =

# Directories
SRC_DIR = src
INC_DIR = include
BUILD_DIR = build
BIN_DIR = .

# Source files
SOURCES = $(wildcard $(SRC_DIR)/*.c)
OBJECTS = $(SOURCES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

# Target executable
TARGET = $(BIN_DIR)/portmap

# Default target
all: $(TARGET)

# Create build directory if it doesn't exist
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Link object files to create executable
# Learning C: The linker combines all .o files into one executable
$(TARGET): $(OBJECTS) | $(BUILD_DIR)
	$(CC) $(OBJECTS) -o $(TARGET) $(LDFLAGS)
	@echo "Build complete: $(TARGET)"

# Compile source files to object files
# Learning C: .o files are "object files" - compiled but not yet linked
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean build artifacts
clean:
	rm -rf $(BUILD_DIR) $(TARGET)
	@echo "Cleaned build files"

# Install to system (requires sudo)
install: $(TARGET)
	install -m 755 $(TARGET) /usr/local/bin/
	@echo "Installed to /usr/local/bin/portmap"

# Uninstall from system
uninstall:
	rm -f /usr/local/bin/portmap
	@echo "Uninstalled portmap"

# Debug build
debug: CFLAGS += -g -DDEBUG
debug: clean all

# Run the program
run: $(TARGET)
	./$(TARGET)

# Help
help:
	@echo "Available targets:"
	@echo "  all       - Build the program (default)"
	@echo "  clean     - Remove build files"
	@echo "  test      - Build and run all unit tests"
	@echo "  checkpatch- Run code style checker on all sources"
	@echo "  install   - Install to /usr/local/bin (requires sudo)"
	@echo "  uninstall - Remove from /usr/local/bin"
	@echo "  debug     - Build with debug symbols"
	@echo "  run       - Build and run the program"

# Test targets
TEST_DIR = tests
TEST_BUILD_DIR = $(BUILD_DIR)/tests
TEST_SOURCES = $(wildcard $(TEST_DIR)/test_*.c)
TEST_BINS = $(TEST_SOURCES:$(TEST_DIR)/test_%.c=$(TEST_BUILD_DIR)/test_%)

# Object files needed for tests (exclude main.o)
TEST_OBJECTS = $(filter-out $(BUILD_DIR)/main.o, $(OBJECTS))

# Create test build directory
$(TEST_BUILD_DIR):
	mkdir -p $(TEST_BUILD_DIR)

# Build individual test executables
$(TEST_BUILD_DIR)/test_%: $(TEST_DIR)/test_%.c $(TEST_OBJECTS) | $(TEST_BUILD_DIR)
	$(CC) $(CFLAGS) $< $(TEST_OBJECTS) -o $@

# Run all tests
test: $(TARGET) $(TEST_BINS)
	@echo ""
	@echo "════════════════════════════════════════"
	@echo "Running All Unit Tests"
	@echo "════════════════════════════════════════"
	@for test in $(TEST_BINS); do \
		$$test || exit 1; \
	done
	@echo ""
	@echo "All test suites passed!"
	@echo ""

# Run individual test
test-%: $(TEST_BUILD_DIR)/test_%
	./$<

# Code style checking
checkpatch:
	@./scripts/check_style.sh

.PHONY: all clean install uninstall debug run help test test-% checkpatch
