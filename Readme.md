# 🍚 kacchiOS

A minimal, educational baremetal operating system designed for teaching OS fundamentals.

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![Build](https://img.shields.io/badge/build-passing-brightgreen.svg)]()
[![Platform](https://img.shields.io/badge/platform-x86-lightgrey.svg)]()

## 📖 Overview

kacchiOS is a simple, bare-metal operating system built from scratch for educational purposes. It provides a clean foundation for students to learn operating system concepts by implementing core components themselves.

### Current Features

- ✅ **Multiboot-compliant bootloader** - Boots via GRUB/QEMU
- ✅ **Memory Manager** - 64KB heap allocation with first-fit algorithm
- ✅ **Process Manager** - PCB-based process control with context switching
- ✅ **Priority Scheduler** - Priority-based scheduling with aging mechanism
- ✅ **Serial I/O driver** (COM1) - Communication via serial port
- ✅ **Clean, documented code** - Easy to understand and extend

## 🚀 Quick Start

### Prerequisites

```bash
# On Ubuntu/Debian
sudo apt-get install build-essential qemu-system-x86 gcc-multilib

# On Arch Linux
sudo pacman -S base-devel qemu gcc-multilib

# On macOS
brew install qemu i686-elf-gcc
```

### Build and Run

```bash
# Clone the repository
git clone https://github.com/enzymesharafot/KacchiOS_X.git
cd KacchiOS_X

# Build the OS
make clean
make

# Run in QEMU
./run.sh
```

You should see:

```
========================================
            kacchiOS_X
========================================
       Hello from kacchiOS!

Initializing OS components...
Memory manager initialized.
Process manager initialized.
All components initialized successfully!

X_Kacchi> 
```

Type commands like `help`, `demo`, `ps`, or `run`!

## 📁 Project Structure

```
kacchiOS/
├── src/
│   ├── boot.S          # Bootloader entry point (Assembly)
│   ├── kernel.c        # Main kernel with shell
│   ├── memory.c/h      # Memory manager implementation
│   ├── process.c/h     # Process manager with scheduler
│   ├── scheduler.c/h   # Scheduler interface
│   ├── ctxsw.S         # Context switching (Assembly)
│   ├── serial.c/h      # Serial port driver (COM1)
│   ├── string.c/h      # String utility functions
│   ├── types.h         # Basic type definitions
│   ├── io.h            # I/O port operations
│   └── link.ld         # Linker script
├── Makefile            # Build system
├── run.sh              # Quick run script (Linux/macOS)
├── run.bat             # Quick run script (Windows)
├── REQUIREMENTS.md     # Detailed setup instructions
└── README.md           # This file
```

## 🛠️ Build System

### Makefile Targets

| Command | Description |
|---------|-------------|
| `make` or `make all` | Build kernel.elf |
| `make clean` | Remove build artifacts |

### Quick Run Scripts

| Platform | Command |
|----------|---------|
| Linux/macOS | `./run.sh` |
| Windows (MSYS2) | `run.bat` |

## 📚 Commands

Available in kacchiOS shell:

- `help` - Show available commands
- `demo` - Create and demonstrate processes
- `run` - Start the process scheduler
- `ps` - List all processes
- `mem` - Show memory information
- `clear` - Clear screen
- `about` - About kacchiOS

## 🎓 Learning Resources

### Recommended Reading

- [OSDev Wiki](https://wiki.osdev.org/) - Comprehensive OS development guide
- [The Little OS Book](https://littleosbook.github.io/) - Practical OS development
- [Operating Systems: Three Easy Pieces](https://pages.cs.wisc.edu/~remzi/OSTEP/) - OS concepts textbook

### Related Topics

- x86 Assembly Language
- Memory Management
- Process Scheduling
- Context Switching
- Interrupt Handling

## 📄 License

This project is licensed under the MIT License.

## 👨‍🏫 About

kacchiOS was created as an educational tool for learning operating system concepts. It provides a minimal, working foundation that demonstrates core OS principles through hands-on implementation.
