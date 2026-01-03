# kacchiOS Quick Start Guide

## 🚀 Build & Run

```bash
# Navigate to project
cd /media/inzamul-sharafot/Data/Os_Project/kacchiOS

# Clean and build
make clean
make

# Run in QEMU (serial output only - recommended)
make run

# Run with VGA window
make run-vga
```

## 📋 Interactive Commands

Once kacchiOS boots, you'll see the prompt:
```
kacchiOS>
```

Available commands:

| Command | Description |
|---------|-------------|
| `help` | Show all available commands |
| `demo` | **Run comprehensive OS demo** (recommended first!) |
| `mem` | Display memory statistics |
| `ps` | Show process table |
| `sched` | Show scheduler statistics |
| `clear` | Clear screen |
| `about` | About kacchiOS |

## 🎯 Quick Demo

```
kacchiOS> demo
```

This will demonstrate:
1. ✅ Memory allocation/deallocation
2. ✅ Process creation (3 processes)
3. ✅ Priority-based scheduling
4. ✅ Context switching
5. ✅ Complete statistics

## 📊 What Gets Tested

### Memory Manager Demo
```
- Allocate 100 bytes
- Allocate 200 bytes  
- Allocate 50 bytes
- Free 200 bytes
- Allocate 150 bytes (reuses freed space)
- Show statistics
```

### Process Manager Demo
```
- Create Process-A (NORMAL priority)
- Create Process-B (NORMAL priority)
- Create Process-C (LOW priority)
- Display process table
- Track state transitions
```

### Scheduler Demo
```
- Add processes to ready queues
- Schedule based on priority
- Perform context switches
- Track quantum usage
- Show final statistics
```

## 📈 Expected Output Example

```
========================================
    kacchiOS - Minimal Baremetal OS
========================================
Hello from kacchiOS!
Enhanced with Memory, Process, and Scheduler!

Initializing OS components...
Memory Manager initialized:
  Stack size: 65536 bytes
  Heap size: 65536 bytes
Process Manager initialized (max 16 processes)
Scheduler initialized (policy: Priority with Round-Robin, quantum: 100)
All components initialized successfully!

kacchiOS> demo

=== kacchiOS Demo ===

=== Testing Memory Manager ===
Allocated 100 bytes at 0x...
Allocated 200 bytes at 0x...
...

=== Creating Processes ===
Created process 'Process-A' (PID 1)
Created process 'Process-B' (PID 2)
Created process 'Process-C' (PID 3)

=== Process Table ===
PID  Name                State       Priority
---  ------------------  ----------  --------
 1   Process-A           READY       NORMAL
 2   Process-B           READY       NORMAL
 3   Process-C           READY       LOW

=== Starting Scheduler ===
[SCHED] Switched to process 'Process-A' (PID 1)
[Process A] Starting...
[Process A] Running iteration 1
...
```

## 🏗️ Component Structure

```
Memory Manager (memory.c/h)
├─ Stack allocation (bump allocator)
├─ Heap allocation (first-fit)
└─ Heap deallocation (with coalescing)

Process Manager (process.c/h)
├─ Process table (16 processes max)
├─ Process creation/termination
└─ State management (READY/CURRENT/TERMINATED)

Scheduler (scheduler.c/h)
├─ Priority-based Round-Robin
├─ Ready queue per priority
└─ Context switching
```

## 🔧 Troubleshooting

### Build Issues
```bash
# If build fails, try:
make clean
make

# Check GCC version
gcc --version  # Should be 32-bit capable

# Check dependencies
which qemu-system-i386
```

### Runtime Issues
```bash
# If QEMU doesn't start:
make run-vga  # Try VGA mode instead

# To exit QEMU:
Ctrl+A, then X  # (serial mode)
Ctrl+C          # (VGA mode)
```

## 📚 Documentation Files

| File | Purpose |
|------|---------|
| `SUMMARY.md` | Quick implementation summary |
| `IMPLEMENTATION.md` | Detailed technical documentation |
| `ARCHITECTURE.md` | Component architecture & flow |
| `QUICKSTART.md` | This file - usage guide |
| `Readme.md` | Original kacchiOS README |

## ✅ Implementation Checklist

### Memory Manager
- [x] Stack allocation
- [x] Heap allocation  
- [x] Heap deallocation
- [x] Block coalescing
- [x] Statistics tracking

### Process Manager
- [x] Process table
- [x] Process creation
- [x] State transitions
- [x] Process termination
- [x] Priority levels

### Scheduler
- [x] Scheduling policy
- [x] Ready queue management
- [x] Context switching
- [x] Time quantum
- [x] Statistics

## 🎓 For Students/Reviewers

### To evaluate the implementation:

1. **Build the project**
   ```bash
   make clean && make
   ```

2. **Run the demo**
   ```bash
   make run
   # Then type: demo
   ```

3. **Verify components**
   ```bash
   # Type these commands:
   mem       # Check memory manager
   ps        # Check process manager
   sched     # Check scheduler
   ```

4. **Review the code**
   - `memory.c` - Memory management logic
   - `process.c` - Process management logic
   - `scheduler.c` - Scheduling logic
   - `kernel.c` - Integration & demo

5. **Check documentation**
   - `IMPLEMENTATION.md` - Technical details
   - `ARCHITECTURE.md` - Design decisions

## 💡 Tips

- The `demo` command showcases everything at once
- Use `help` to see all available commands
- All components log their operations for visibility
- Statistics are updated in real-time
- The implementation is educational - code is well-commented

## 🚀 Next Steps

After testing the basic demo:

1. Examine the source code to understand implementation
2. Read IMPLEMENTATION.md for detailed explanations
3. Try creating more processes (modify kernel.c)
4. Experiment with different priorities
5. Add your own features!

---

**Ready to test?**
```bash
cd /media/inzamul-sharafot/Data/Os_Project/kacchiOS
make clean && make
make run
# Type: demo
```

Enjoy exploring kacchiOS! 🎉
