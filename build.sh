#!/bin/bash

# Set the output binary name
OUTPUT="chip8_emulator"

# Source files
SRC_FILES="chip8.c main.c"

# Compiler and flags
CC=gcc
CFLAGS="-Wall -Wextra -pedantic -std=c99"
LDFLAGS="-lGL -lGLU -lglut -lm"

# Compile the program
echo "Compiling CHIP-8 Emulator..."
$CC $CFLAGS $SRC_FILES -o $OUTPUT $LDFLAGS

# Check if compilation was successful
if [ $? -eq 0 ]; then
    echo "Compilation successful! Run the emulator with:"
    echo "./$OUTPUT <path_to_rom>"
else
    echo "Compilation failed. Check errors above."
    exit 1
fi
