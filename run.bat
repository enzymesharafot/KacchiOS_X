@echo off
REM kacchiOS Build and Run Script for Windows (MSYS2)

echo ==================================
echo   kacchiOS Build ^& Run Script
echo ==================================
echo.

REM Clean previous build
echo [1/3] Cleaning previous build...
make clean

REM Build the kernel
echo.
echo [2/3] Building kernel...
make

REM Check if build was successful
if %ERRORLEVEL% EQU 0 (
    echo.
    echo [3/3] Launching kacchiOS in QEMU...
    echo Press Ctrl+A then X to exit QEMU
    echo.
    timeout /t 1 >nul
    qemu-system-i386 -kernel kernel.elf -m 64M -serial stdio -display none
) else (
    echo.
    echo Build failed! Please check the errors above.
    exit /b 1
)
