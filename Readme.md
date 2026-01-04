# kacchiOS_X

A baremetal operating system for x86 architecture.

## Features

- Memory Manager (64KB heap allocation)
- Process Manager (PCB-based process control)
- Priority Scheduler (with aging mechanism)
- Context Switching
- Serial I/O (COM1)

## Build Requirements

```bash
# Ubuntu/Debian
sudo apt-get install build-essential qemu-system-x86 gcc-multilib

# Arch Linux
sudo pacman -S base-devel qemu gcc-multilib
```

## Build & Run

```bash
# Build
make clean && make

# Run
qemu-system-i386 -kernel kernel.elf -m 64M -serial stdio -display none
```

## Project Structure

```
kacchiOS/
├── src/
│   ├── boot.S           # Boot entry
│   ├── kernel.c         # Main kernel
│   ├── memory.c/h       # Memory manager
│   ├── process.c/h      # Process manager
│   ├── scheduler.c/h    # Scheduler
│   ├── ctxsw.S          # Context switch
│   ├── serial.c/h       # Serial driver
│   ├── string.c/h       # String utilities
│   └── types.h          # Type definitions
├── Makefile
└── README.md
```

## Commands

- `help` - Show available commands
- `demo` - Create demo processes
- `run` - Start scheduler
- `ps` - List processes
- `mem` - Memory info

## License

MIT License
