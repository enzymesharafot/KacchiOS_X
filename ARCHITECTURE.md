# kacchiOS Component Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                         KERNEL (kernel.c)                        │
│                      Interactive Shell + Demo                    │
└────────────┬────────────────┬────────────────┬──────────────────┘
             │                │                │
             ▼                ▼                ▼
    ┌────────────────┐ ┌─────────────┐ ┌──────────────┐
    │ Memory Manager │ │   Process   │ │  Scheduler   │
    │  (memory.c/h)  │ │   Manager   │ │(scheduler.c) │
    │                │ │(process.c/h)│ │              │
    │ • Stack Alloc  │ │ • PCB Table │ │ • Priority   │
    │ • Heap Alloc   │ │ • Create    │ │ • RR Policy  │
    │ • Heap Free    │ │ • Terminate │ │ • Context    │
    │ • Statistics   │ │ • States    │ │   Switch     │
    └────────┬───────┘ └──────┬──────┘ └──────┬───────┘
             │                │                │
             └────────────────┼────────────────┘
                              │
                    ┌─────────┴──────────┐
                    │  Support Libraries  │
                    │                     │
                    │ • serial.c/h        │
                    │   (I/O operations)  │
                    │                     │
                    │ • string.c/h        │
                    │   (String utils)    │
                    │                     │
                    │ • types.h           │
                    │   (Type defs)       │
                    └────────────────────┘
```

## Component Interaction Flow

### 1. System Initialization
```
kernel.c:kmain()
    │
    ├──> serial_init()         [Initialize I/O]
    │
    ├──> mem_init()            [Initialize Memory Manager]
    │       └──> Setup stack region
    │       └──> Setup heap region
    │
    ├──> proc_init()           [Initialize Process Manager]
    │       └──> Clear process table
    │
    └──> sched_init()          [Initialize Scheduler]
            └──> Setup ready queues
```

### 2. Process Creation Flow
```
proc_create()
    │
    ├──> Find free PCB slot
    │
    ├──> mem:stack_alloc()     [Allocate stack from Memory Manager]
    │
    ├──> Initialize PCB
    │       ├── Set PID
    │       ├── Set name
    │       ├── Set priority
    │       └── Set state = READY
    │
    └──> Return PID
```

### 3. Scheduling Flow
```
sched_schedule()
    │
    ├──> Select next process (Priority + RR)
    │       ├── Check HIGH priority queue
    │       ├── Check NORMAL priority queue
    │       └── Check LOW priority queue
    │
    ├──> sched_switch_context()
    │       ├── Save old process state
    │       │       └── proc:state = READY
    │       │
    │       └── Load new process state
    │               ├── proc:state = CURRENT
    │               └── Reset time quantum
    │
    └──> Execute process
```

### 4. Memory Allocation Flow
```
heap_alloc(size)
    │
    ├──> Align size to 4-byte boundary
    │
    ├──> Find free block (First-fit)
    │       └── Walk linked list
    │
    ├──> Split block if too large
    │       └── Create new free block
    │
    ├──> Mark block as allocated
    │
    ├──> Update statistics
    │
    └──> Return pointer
```

### 5. Memory Deallocation Flow
```
heap_free(ptr)
    │
    ├──> Get block header
    │
    ├──> Validate (check double-free)
    │
    ├──> Mark block as free
    │
    ├──> Update statistics
    │
    └──> Merge adjacent free blocks
            ├── Check previous block
            └── Check next block
```

## Data Structures

### Memory Block (mem_block_t)
```
┌─────────────────────────┐
│ size: 1024              │ ← Size of usable area
├─────────────────────────┤
│ is_free: 0/1            │ ← Allocation flag
├─────────────────────────┤
│ next: ptr               │ ← Next block pointer
├─────────────────────────┤
│ [User Data Area]        │ ← Allocated memory
│ ...                     │
└─────────────────────────┘
```

### Process Control Block (pcb_t)
```
┌─────────────────────────┐
│ pid: 1                  │
├─────────────────────────┤
│ name: "Process-A"       │
├─────────────────────────┤
│ state: READY/CURRENT    │
├─────────────────────────┤
│ priority: NORMAL        │
├─────────────────────────┤
│ context:                │
│   ├─ esp (stack ptr)    │
│   ├─ ebp (base ptr)     │
│   ├─ eip (instr ptr)    │
│   └─ eflags             │
├─────────────────────────┤
│ stack_base: ptr         │
│ stack_size: 4096        │
├─────────────────────────┤
│ time_quantum: 100       │
│ total_runtime: 542      │
└─────────────────────────┘
```

### Ready Queue (per priority)
```
HIGH Priority:    [P1] → [P4] → NULL
NORMAL Priority:  [P2] → [P5] → [P7] → NULL
LOW Priority:     [P3] → [P6] → NULL
```

## File Structure

```
kacchiOS/
├── Core Components (NEW)
│   ├── memory.h           # Memory Manager interface
│   ├── memory.c           # Memory Manager implementation
│   ├── process.h          # Process Manager interface
│   ├── process.c          # Process Manager implementation
│   ├── scheduler.h        # Scheduler interface
│   └── scheduler.c        # Scheduler implementation
│
├── Kernel
│   ├── kernel.c           # Main kernel (MODIFIED)
│   ├── boot.S             # Boot assembly
│   └── link.ld            # Linker script
│
├── Support Libraries (MODIFIED)
│   ├── serial.c/h         # I/O (added print functions)
│   ├── string.c/h         # String utils (added memset, strncpy)
│   ├── types.h            # Type definitions
│   └── io.h               # I/O port operations
│
├── Build System
│   └── Makefile           # Build rules (MODIFIED)
│
└── Documentation (NEW)
    ├── IMPLEMENTATION.md  # Detailed technical docs
    ├── SUMMARY.md         # Quick reference
    ├── ARCHITECTURE.md    # This file
    └── Readme.md          # Original README
```

## Execution Flow Example

```
User runs: kacchiOS> demo

1. test_memory()
   ├─> heap_alloc(100)      [Memory Manager]
   ├─> heap_alloc(200)
   ├─> heap_free(200)
   └─> mem_print_stats()

2. proc_create("Process-A", ...) [Process Manager]
   ├─> Allocate PID = 1
   ├─> stack_alloc(4096)    [Memory Manager]
   └─> State = READY

3. proc_create("Process-B", ...)
   └─> PID = 2

4. proc_create("Process-C", ...)
   └─> PID = 3, Priority = LOW

5. sched_add_ready(PCB_A)   [Scheduler]
   └─> Add to NORMAL queue

6. sched_add_ready(PCB_B)
   └─> Add to NORMAL queue

7. sched_add_ready(PCB_C)
   └─> Add to LOW queue

8. Loop: sched_schedule()
   ├─> Select Process-A (NORMAL queue)
   ├─> Context switch
   ├─> Process-A runs
   ├─> sched_yield()
   ├─> Select Process-B
   ├─> Context switch
   ├─> Process-B runs
   └─> ... continues ...

9. Processes terminate
   └─> proc_terminate()
       └─> State = TERMINATED

10. Print statistics
    ├─> proc_print_table()
    ├─> sched_print_stats()
    └─> mem_print_stats()
```

## Key Design Decisions

### 1. Memory Manager
- **Split allocation**: Stack (bump) + Heap (free list)
  - Reason: Simple stack allocation for predictable cases
  - Heap for dynamic allocation needs
- **First-fit algorithm**: Balance between speed and fragmentation
- **Coalescing**: Reduces external fragmentation

### 2. Process Manager  
- **Fixed process table**: Array-based for simplicity
  - Alternative: Dynamic linked list (more flexible but complex)
- **Priority levels**: Three levels sufficient for demonstration
- **Stack per process**: Isolation and safety

### 3. Scheduler
- **Priority + Round-Robin**: Combines fairness with priority
- **Multiple queues**: One per priority level
- **Configurable quantum**: Allows tuning for workload

## Statistics & Monitoring

All components provide statistics:

```
Memory Stats:
├─ Total heap size
├─ Used heap size
├─ Free heap size
├─ Number of allocations
└─ Number of deallocations

Process Stats:
├─ Active process count
├─ Per-process: PID, name, state, priority
└─ Process table utilization

Scheduler Stats:
├─ Total context switches
├─ Total scheduler calls
├─ Idle time
└─ Current policy
```

## Testing Commands

```bash
# Build
make clean && make

# Run with serial output
make run

# Interactive testing
kacchiOS> demo      # Run full demo
kacchiOS> mem       # Check memory
kacchiOS> ps        # List processes  
kacchiOS> sched     # Scheduler stats
```

---

**Implementation Complete**: All components working together seamlessly!
