#!/usr/bin/bash

echo "Compiling $1 with SIMD optimizations..."

# Detect CPU architecture for best SIMD instruction set
if grep -q "avx512" /proc/cpuinfo; then
    SIMD_FLAGS="-mavx512f -mavx512dq"
    echo "Using AVX-512 instructions"
elif grep -q "avx2" /proc/cpuinfo; then
    SIMD_FLAGS="-mavx2 -mfma"
    echo "Using AVX2 instructions"
elif grep -q "avx" /proc/cpuinfo; then
    SIMD_FLAGS="-mavx"
    echo "Using AVX instructions"
elif grep -q "sse4_2" /proc/cpuinfo; then
    SIMD_FLAGS="-msse4.2"
    echo "Using SSE4.2 instructions"
else
    SIMD_FLAGS="-msse2"
    echo "Using SSE2 instructions (minimum)"
fi

# Compile with optimizations and SIMD flags
gcc $1.c Mathutils.c -o $1 \
    -lSDL2 -lSDL2_image $(sdl2-config --cflags --libs) -lm \
    -Wall -Wextra -fopenmp \
    -O3 -march=native ${SIMD_FLAGS} \
    -ffast-math -ftree-vectorize -funroll-loops

echo "Compilation complete: $1"
