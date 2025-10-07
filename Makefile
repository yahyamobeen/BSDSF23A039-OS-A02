# Makefile for ls utility

# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -std=gnu99
SRCDIR = src
OBJDIR = obj
BINDIR = bin

# Source files
SOURCES = $(SRCDIR)/ls-v1.0.0.c
OBJECTS = $(OBJDIR)/ls-v1.0.0.o
TARGET = $(BINDIR)/ls

# Default target
all: $(TARGET)

# Create binary
$(TARGET): $(OBJECTS) | $(BINDIR)
	$(CC) $(OBJECTS) -o $(TARGET)

# Create object files
$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Create directories
$(BINDIR):
	mkdir -p $(BINDIR)

$(OBJDIR):
	mkdir -p $(OBJDIR)

# Clean build artifacts
clean:
	rm -rf $(OBJDIR)/* $(BINDIR)/*

# Install (optional)
install: all
	cp $(TARGET) /usr/local/bin/

.PHONY: all clean install
