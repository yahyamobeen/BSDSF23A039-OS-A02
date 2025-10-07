# Makefile for ls utility

CC = gcc
CFLAGS = -Wall -Wextra -g
TARGET = bin/ls
SRC = src/ls-v1.0.0.c
OBJ = obj/ls-v1.0.0.o

# Default target
all: $(TARGET)

# Create directories if they don't exist
$(TARGET): $(OBJ)
	@mkdir -p bin
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ)

$(OBJ): $(SRC)
	@mkdir -p obj
	$(CC) $(CFLAGS) -c $(SRC) -o $(OBJ)

# Clean build artifacts
clean:
	rm -rf bin/* obj/*

# Phony targets
.PHONY: all clean
