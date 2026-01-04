/* process.h - Process Manager Interface (XINU Style) */
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

/* Process Control Block (PCB) - XINU Style */
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

/* Process manager functions */
void proc_init(void);
void proc_run(void);
int32_t proc_create(void (*func)(void));
void proc_exit(void);
void proc_list(void);

/* Scheduler functions */
void resched(void);
void yield(void);
void aging_update(void);

/* Sleep and wait functions */
void sleep(int ticks);
void proc_tick(void);
void wait(int event);
void wakeup(int event);

/* Memory allocation (from memory.c) */
void *mem_alloc(size_t size);
void mem_free(void *ptr);

#endif
