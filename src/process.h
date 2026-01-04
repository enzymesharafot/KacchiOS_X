/* process.h - Process Manager Interface*/
#ifndef PROCESS_H
#define PROCESS_H

#include "types.h"

/* Maximum number of processes */
#define MAX_PROCS 16

/* Process states */
typedef enum {
    PR_TERMINATED,  /* Process has terminated */
    PR_CURRENT,     /* Process is currently running */
    PR_READY,       /* Process is ready to run */
    PR_SLEEP,       /* Process is sleeping */
    PR_WAIT         /* Process is waiting for event */
} proc_state_t;

/* Process Control Block (PCB) */
typedef struct {
    int32_t pid;           /* Process ID */
    proc_state_t state;    /* Current state */
    void (*entry)(void);   /* Entry point function */
    void *stack_base;      /* Stack base address */
    uint32_t *esp;         /* Saved stack pointer */
    void *mem;             /* Allocated memory pointer */
    uint32_t memsz;        /* Memory size */
    int sleep_ticks;       /* Ticks remaining for sleep */
    int wait_event;        /* Event ID for wait */
    int priority;          /* Base priority */
    int dyn_priority;      /* Dynamic priority (for aging) */
} pcb_t;

/* Global current process pointer */
extern pcb_t *currpid;

/* Global process table (for checking process state) */
extern pcb_t proctab[MAX_PROCS];

/* Process manager functions */
void process_manager_initialize(void);
void process_scheduler_start(void);
int32_t process_create(void (*func)(void));
void process_terminate(void);
void process_list_display(void);

#endif
