# Compiler and flags
# No vs code, utilizar crtl+shift+b
CC        := gcc-13
CFLAGS    := -Wall -Wextra -g  # Added warning flags for better code quality

# Directory structure
BIN      := bin
SRC      := src
INCLUDE  := include

# Libraries and executable name
EXECUTABLE := server

# Main target
all: $(BIN)/$(EXECUTABLE)

# Run target - cleans, rebuilds, and executes the program
run: clean all
	clear
	./$(BIN)/$(EXECUTABLE)

# Compilation rule
# Note: $^ represents all prerequisites (source files)
# Note: $@ represents the target (the executable)
$(BIN)/$(EXECUTABLE): $(SRC)/*.c
	$(CC) $(CFLAGS) -I$(INCLUDE) $^ -o $@

# Clean rule - removes compiled files
# The - before rm makes make continue even if there's nothing to remove
clean:
	-rm $(BIN)/*