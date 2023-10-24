#!/bin/bash

# Create build directory and generate Makefiles
cmake -B build -DCMAKE_BUILD_TYPE=Release 

# Build the project
cmake --build build

# Run the calc program and pipe the output to llc
build/src/calc  "with a : int b : b = a + 3 : with m : a = b + a * 10 - m" | llc -filetype=obj -relocation-model=pic -o exp.o

# Compile the object file using clang++
clang++ -g -o exp exp.o rtcalc.cpp

# Run the compiled program with gdb for debugging
./exp


