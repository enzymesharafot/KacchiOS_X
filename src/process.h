/* process.h - Process Manager Interface */
#ifndef PROCESS_H
#define PROCESS_H

#include "../types.h"

/* Process states */
typedef enum {
    PROC_TERMINATED,  /* Process has terminated */
    PROC_CURRENT,     /* Process is currently running */
    PROC_READY        /* Process is ready to run */
} proc_state_t;

/* Process priority levels */
typedef enum {
    PRIO_HIGH = 0,
    PRIO_NORMAL = 1,
    PRIO_LOW = 2
} proc_priority_t;

/* Process context (saved registers during context switch) */
typedef struct {
    uint32_t esp;     /* Stack pointer */
    uint32_t ebp;     /* Base pointer */
    uint32_t ebx;     /* General purpose registers */
    uint32_t esi;
    uint32_t edi;
    uint32_t eflags;  /* CPU flags */
    uint32_t eip;     /* Instruction pointer */
} proc_context_t;

/* Process Control Block (PCB) */
typedef struct {
    uint32_t pid;                  /* Process ID */
    char name[32];                 /* Process name */
    proc_state_t state;            /* Current state */
    proc_priority_t priority;      /* Process priority */
    proc_context_t context;        /* Saved context */
    void *stack_base;              /* Stack base address */
    size_t stack_size;             /* Stack size */
    uint32_t time_quantum;         /* Time quantum remaining */
    uint32_t total_runtime;        /* Total CPU time used */
} pcb_t;

/* Maximum number of processes */
#define MAX_PROCESSES 16

/* Process manager initialization */
void proc_init(void);

/* Create a new process */
int proc_create(const char *name, void (*entry_point)(void), 
                proc_priority_t priority, size_t stack_size);

/* Terminate a process */
void proc_terminate(uint32_t pid);

/* Get process by PID */
pcb_t *proc_get(uint32_t pid);

/* Get current running process */
pcb_t *proc_current(void);

/* Get process count by state */
int proc_count_by_state(proc_state_t state);

/* Print process table */
void proc_print_table(void);

/* Utility functions to get process specific data */
uint32_t proc_get_pid(void);
const char *proc_state_to_string(proc_state_t state);

#endif
