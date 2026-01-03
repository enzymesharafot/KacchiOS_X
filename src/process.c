/* process.c - Process Manager Implementation */
#include "process.h"
#include "memory.h"
#include "../string.h"
#include "../serial.h"

/* Process table */
static pcb_t process_table[MAX_PROCESSES];
static uint32_t next_pid = 0;
static uint32_t current_pid = 0;
static int num_processes = 0;

/* Initialize the process manager */
void proc_init(void) {
    /* Clear process table */
    memset(process_table, 0, sizeof(process_table));
    
    /* Initialize all processes as terminated */
    for (int i = 0; i < MAX_PROCESSES; i++) {
        process_table[i].state = PROC_TERMINATED;
        process_table[i].pid = 0;
    }
    
    next_pid = 1;
    current_pid = 0;
    num_processes = 0;
    
    serial_puts("Process Manager initialized (max ");
    serial_put_uint(MAX_PROCESSES);
    serial_puts(" processes)\n");
}

/* Find a free slot in the process table */
static int find_free_slot(void) {
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i].state == PROC_TERMINATED) {
            return i;
        }
    }
    return -1;
}

/* Initialize process stack with context */
static void init_process_stack(pcb_t *pcb, void (*entry_point)(void)) {
    /* Allocate stack for the process */
    pcb->stack_base = stack_alloc(pcb->stack_size);
    if (pcb->stack_base == NULL) {
        serial_puts("ERROR: Failed to allocate stack for process\n");
        return;
    }
    
    /* Set up initial stack pointer (stack grows downward) */
    uint32_t *stack_top = (uint32_t *)((uint8_t *)pcb->stack_base + pcb->stack_size);
    
    /* Push initial values onto stack for context switch */
    /* When we context switch to this process, it will "return" to entry_point */
    stack_top--;
    *stack_top = (uint32_t)entry_point;  /* Return address (eip) */
    
    stack_top--;
    *stack_top = 0x00000202;  /* EFLAGS (interrupts enabled) */
    
    stack_top--;
    *stack_top = 0;  /* EDI */
    
    stack_top--;
    *stack_top = 0;  /* ESI */
    
    stack_top--;
    *stack_top = 0;  /* EBX */
    
    stack_top--;
    *stack_top = 0;  /* EBP */
    
    /* Set initial context */
    pcb->context.esp = (uint32_t)stack_top;
    pcb->context.ebp = (uint32_t)pcb->stack_base + pcb->stack_size;
    pcb->context.ebx = 0;
    pcb->context.esi = 0;
    pcb->context.edi = 0;
    pcb->context.eflags = 0x00000202;
    pcb->context.eip = (uint32_t)entry_point;
}

/* Create a new process */
int proc_create(const char *name, void (*entry_point)(void), 
                proc_priority_t priority, size_t stack_size) {
    if (num_processes >= MAX_PROCESSES) {
        serial_puts("ERROR: Process table full\n");
        return -1;
    }
    
    if (entry_point == NULL) {
        serial_puts("ERROR: Invalid entry point\n");
        return -1;
    }
    
    if (stack_size < 1024) {
        stack_size = 1024;  /* Minimum stack size */
    }
    
    /* Find a free slot */
    int slot = find_free_slot();
    if (slot < 0) {
        serial_puts("ERROR: No free process slot\n");
        return -1;
    }
    
    pcb_t *pcb = &process_table[slot];
    
    /* Initialize PCB */
    pcb->pid = next_pid++;
    strncpy(pcb->name, name, sizeof(pcb->name) - 1);
    pcb->name[sizeof(pcb->name) - 1] = '\0';
    pcb->state = PROC_READY;
    pcb->priority = priority;
    pcb->stack_size = stack_size;
    pcb->time_quantum = 100;  /* Default time quantum */
    pcb->total_runtime = 0;
    
    /* Initialize stack and context */
    init_process_stack(pcb, entry_point);
    
    num_processes++;
    
    serial_puts("Created process '");
    serial_puts(pcb->name);
    serial_puts("' (PID ");
    serial_put_uint(pcb->pid);
    serial_puts(")\n");
    
    return pcb->pid;
}

/* Terminate a process */
void proc_terminate(uint32_t pid) {
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i].pid == pid && 
            process_table[i].state != PROC_TERMINATED) {
            
            serial_puts("Terminating process '");
            serial_puts(process_table[i].name);
            serial_puts("' (PID ");
            serial_put_uint(pid);
            serial_puts(")\n");
            
            /* Mark as terminated */
            process_table[i].state = PROC_TERMINATED;
            process_table[i].pid = 0;
            
            /* Note: Stack memory is not freed as we use bump allocator */
            /* In a real OS, you'd free the stack memory here */
            
            num_processes--;
            return;
        }
    }
    
    serial_puts("WARNING: Process PID ");
    serial_put_uint(pid);
    serial_puts(" not found\n");
}

/* Get process by PID */
pcb_t *proc_get(uint32_t pid) {
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i].pid == pid && 
            process_table[i].state != PROC_TERMINATED) {
            return &process_table[i];
        }
    }
    return NULL;
}

/* Get current running process */
pcb_t *proc_current(void) {
    if (current_pid == 0) {
        return NULL;
    }
    return proc_get(current_pid);
}

/* Get current process PID */
uint32_t proc_get_pid(void) {
    return current_pid;
}

/* Set current process (used by scheduler) */
void proc_set_current(uint32_t pid) {
    current_pid = pid;
}

/* Get process count by state */
int proc_count_by_state(proc_state_t state) {
    int count = 0;
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i].state == state) {
            count++;
        }
    }
    return count;
}

/* Convert process state to string */
const char *proc_state_to_string(proc_state_t state) {
    switch (state) {
        case PROC_TERMINATED: return "TERMINATED";
        case PROC_CURRENT: return "CURRENT";
        case PROC_READY: return "READY";
        default: return "UNKNOWN";
    }
}

/* Print process table */
void proc_print_table(void) {
    serial_puts("\n=== Process Table ===\n");
    serial_puts("PID  Name                State       Priority\n");
    serial_puts("---  ------------------  ----------  --------\n");
    
    int active_count = 0;
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i].state != PROC_TERMINATED) {
            pcb_t *pcb = &process_table[i];
            
            /* PID */
            if (pcb->pid < 10) serial_putc(' ');
            serial_put_uint(pcb->pid);
            serial_puts("  ");
            
            /* Name (truncate if too long) */
            int name_len = 0;
            while (pcb->name[name_len] && name_len < 18) {
                serial_putc(pcb->name[name_len]);
                name_len++;
            }
            while (name_len < 18) {
                serial_putc(' ');
                name_len++;
            }
            serial_puts("  ");
            
            /* State */
            const char *state_str = proc_state_to_string(pcb->state);
            serial_puts(state_str);
            int state_len = strlen(state_str);
            while (state_len < 10) {
                serial_putc(' ');
                state_len++;
            }
            serial_puts("  ");
            
            /* Priority */
            switch (pcb->priority) {
                case PRIO_HIGH: serial_puts("HIGH"); break;
                case PRIO_NORMAL: serial_puts("NORMAL"); break;
                case PRIO_LOW: serial_puts("LOW"); break;
            }
            serial_puts("\n");
            
            active_count++;
        }
    }
    
    serial_puts("\nActive processes: ");
    serial_put_uint(active_count);
    serial_puts(" / ");
    serial_put_uint(MAX_PROCESSES);
    serial_puts("\n");
}
