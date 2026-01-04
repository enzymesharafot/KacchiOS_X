# kacchiOS Requirements

## Linux/macOS

### Ubuntu/Debian
```bash
sudo apt-get update
sudo apt-get install build-essential qemu-system-x86 gcc-multilib
```

### Arch Linux
```bash
sudo pacman -S base-devel qemu gcc-multilib
```

### macOS
```bash
brew install qemu i686-elf-gcc
```

## Windows

### Option 1: WSL2 (Recommended)

1. **Install WSL2**
   ```powershell
   wsl --install
   ```

2. **Install Ubuntu from Microsoft Store**

3. **Inside WSL2, install dependencies:**
   ```bash
   sudo apt-get update
   sudo apt-get install build-essential qemu-system-x86 gcc-multilib
   ```

4. **Navigate to project and run:**
   ```bash
   cd /mnt/c/path/to/kacchiOS
   ./run.sh
   ```

### Option 2: MSYS2

1. **Install MSYS2** from https://www.msys2.org/

2. **Open MSYS2 MinGW 32-bit terminal**

3. **Install dependencies:**
   ```bash
   pacman -S mingw-w64-i686-gcc mingw-w64-i686-binutils make qemu
   ```

4. **Navigate to project and build:**
   ```bash
   make clean && make
   qemu-system-i386 -kernel kernel.elf -m 64M -serial stdio -display none
   ```

### Option 3: Docker

1. **Install Docker Desktop** from https://www.docker.com/products/docker-desktop

2. **Create Dockerfile in project root:**
   ```dockerfile
   FROM ubuntu:22.04
   RUN apt-get update && apt-get install -y \
       build-essential \
       qemu-system-x86 \
       gcc-multilib
   WORKDIR /kacchiOS
   ```

3. **Build and run:**
   ```bash
   docker build -t kacchios .
   docker run -it -v ${PWD}:/kacchiOS kacchios bash
   # Inside container:
   make clean && make
   qemu-system-i386 -kernel kernel.elf -m 64M -serial stdio -display none
   ```

## Required Tools

- **GCC** (i686 cross-compiler or multilib support)
- **GNU Binutils** (ld, as)
- **Make**
- **QEMU** (qemu-system-i386)

## Verify Installation

```bash
# Check GCC
gcc --version

# Check Make
make --version

# Check QEMU
qemu-system-i386 --version
```

## Quick Start

After installing requirements:

```bash
# Build
make clean && make

# Run
./run.sh
```

Or manually:
```bash
qemu-system-i386 -kernel kernel.elf -m 64M -serial stdio -display none
```
