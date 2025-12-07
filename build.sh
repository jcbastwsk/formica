#!/bin/sh

# Build script for Formica

echo "Building Formica..."

# Create build directory
mkdir -p build
cd build

# Configure with CMake
cmake ..

# Build
make -j$(sysctl -n hw.ncpu)

if [ $? -eq 0 ]; then
    echo "Build successful! Run with: ./build/formica"
else
    echo "Build failed!"
    exit 1
fi