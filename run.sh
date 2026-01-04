#!/bin/bash

# kacchiOS Build and Run Script

echo "=================================="
echo "  kacchiOS Build & Run Script"
echo "=================================="
echo ""

# Clean previous build
echo "[1/3] Cleaning previous build..."
make clean

# Build the kernel
echo ""
echo "[2/3] Building kernel..."
make

# Check if build was successful
if [ $? -eq 0 ]; then
    echo ""
    echo "[3/3] Launching kacchiOS in QEMU..."
    echo "Press Ctrl+A then X to exit QEMU"
    echo ""
    sleep 1
    qemu-system-i386 -kernel kernel.elf -m 64M -serial stdio -display none
else
    echo ""
    echo "Build failed! Please check the errors above."
    exit 1
fi
