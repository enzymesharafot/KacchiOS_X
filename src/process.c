/* process.c - Process Manager Implementation */
#include "process.h"
#include "serial.h"
#include "memory.h"

#define PROC_STACK_SIZE 4096

pcb_t proctab[MAX_PROCS];  /* Global process table */
static int32_t current_pid = -1;
pcb_t *currpid = NULL;

/* -------------------------------------------------- */
/* Utility                                            */
/* -------------------------------------------------- */

static void serial_put_int(int32_t num) {
    char buf[12];
    int i = 0;

    if (num == 0) {
        serial_putc('0');
        return;
    }

    if (num < 0) {
        serial_putc('-');
        num = -num;
    }

    while (num > 0) {
        buf[i++] = (num % 10) + '0';
        num /= 10;
    }

    while (i--) {
        serial_putc(buf[i]);
    }
}

/* -------------------------------------------------- */
/* SCHEDULER CODE */
/* -------------------------------------------------- */

/* Context switching with stack management */
extern void ctxsw(uint32_t **old, uint32_t **new);

void process_create_with_stack(void (*func)(void)) {
    int available_pid;
    
    for (available_pid = 0; available_pid < MAX_PROCS; available_pid++) {
        if (proctab[available_pid].state == PR_TERMINATED)
            break;
    }
    
    if (available_pid == MAX_PROCS)
        return;
    
    /* Allocate stack for process */
    uint32_t *process_stack = memory_allocate(PROC_STACK_SIZE);
    if (!process_stack) {
        serial_puts("Stack allocation failed.\n");
        return;
    }
    
    /* Set up stack pointer at top of stack */
    uint32_t *stack_pointer = (uint32_t *)((uint32_t)process_stack + PROC_STACK_SIZE);
    stack_pointer = (uint32_t *)((uint32_t)stack_pointer & ~0xF);  // 16-byte align
    
    /* Set up stack as if process was context-switched out */
    *--stack_pointer = (uint32_t)process_terminate;  // Return address when func returns
    *--stack_pointer = (uint32_t)func;               // Return address (where process starts)
    *--stack_pointer = 0;                            // EBP
    *--stack_pointer = 0;                            // EBX
    *--stack_pointer = 0;                            // ESI
    *--stack_pointer = 0;                            // EDI
    *--stack_pointer = 0x0200;                       // EFLAGS (interrupts enabled)
    
    proctab[available_pid].pid = available_pid;
    proctab[available_pid].state = PR_READY;
    proctab[available_pid].entry = func;
    proctab[available_pid].stack_base = process_stack;
    proctab[available_pid].esp = stack_pointer;
    proctab[available_pid].mem = process_stack;
    proctab[available_pid].memsz = PROC_STACK_SIZE;
    proctab[available_pid].priority = 1;
    proctab[available_pid].dyn_priority = 1;
}

void scheduler_reschedule(void) {
    int previous_pid = current_pid;
    int next_pid = -1;
    int highest_priority = -1;
    
    /* Find highest priority READY process using round-robin for ties */
    int start_search = (current_pid + 1) % MAX_PROCS;
    for (int count = 0; count < MAX_PROCS; count++) {
        int i = (start_search + count) % MAX_PROCS;
        if (proctab[i].state == PR_READY) {
            if (proctab[i].dyn_priority > highest_priority) {
                highest_priority = proctab[i].dyn_priority;
                next_pid = i;
            }
        }
    }
    
    /* No READY process found */
    if (next_pid == -1) {
        if (previous_pid >= 0 && proctab[previous_pid].state == PR_CURRENT)
            return;
        next_pid = 0;  // Fall back to idle process
    }
    
    /* Reset priority of scheduled process */
    proctab[next_pid].dyn_priority = proctab[next_pid].priority;
    
    /* Same process, no switch needed */
    if (next_pid == previous_pid && previous_pid >= 0)
        return;
    
    /* Update states */
    if (previous_pid >= 0 && proctab[previous_pid].state == PR_CURRENT)
        proctab[previous_pid].state = PR_READY;
    
    proctab[next_pid].state = PR_CURRENT;
    current_pid = next_pid;
    currpid = &proctab[next_pid];
    
    /* Context switch between processes */
    ctxsw(&proctab[previous_pid].esp, &proctab[next_pid].esp);
}

void process_yield_cpu(void) {
    if (currpid)
        currpid->state = PR_READY;
    scheduler_reschedule();
}

void scheduler_update_aging(void) {
    /* Increase priority of waiting processes to prevent starvation */
    for (int i = 0; i < MAX_PROCS; i++) {
        if (proctab[i].state == PR_READY) {
            proctab[i].dyn_priority++;
        }
    }
}

void process_sleep(int tick_count) {
    if (tick_count <= 0 || currpid == NULL) return;
    currpid->sleep_ticks = tick_count;
    currpid->state = PR_SLEEP;
    scheduler_reschedule();
}

void process_timer_tick(void) {
    for (int i = 0; i < MAX_PROCS; i++) {
        if (proctab[i].state == PR_SLEEP) {
            proctab[i].sleep_ticks--;
            if (proctab[i].sleep_ticks <= 0) {
                proctab[i].state = PR_READY;
            }
        }
    }
}

void process_wait_event(int event_id) {
    currpid->wait_event = event_id;
    currpid->state = PR_WAIT;
    scheduler_reschedule();
}

void process_wakeup_event(int event_id) {
    for (int i = 0; i < MAX_PROCS; i++) {
        if (proctab[i].state == PR_WAIT &&
            proctab[i].wait_event == event_id) {
            proctab[i].wait_event = -1;
            proctab[i].state = PR_READY;
        }
    }
}


/* -------------------------------------------------- */
/* Process Manager Init                               */
/* -------------------------------------------------- */

void process_scheduler_start(void) {
    serial_puts("\n=== Running Processes Sequentially ===\n\n");
    
    /* Run each ready process to completion */
    for (int i = 0; i < MAX_PROCS; i++) {
        if (proctab[i].state == PR_READY && proctab[i].entry != NULL) {
            serial_puts("Starting process ");
            serial_put_int(i);
            serial_puts("...\n");
            
            proctab[i].state = PR_CURRENT;
            current_pid = i;
            currpid = &proctab[i];
            
            /* Call the process function directly */
            proctab[i].entry();
            
            /* Mark as completed */
            proctab[i].state = PR_TERMINATED;
            
            serial_puts("\n");
        }
    }
    
    serial_puts("=== All Processes Completed! ===\n");
    serial_puts("Returning to shell...\n\n");
    
    /* Return to shell instead of hanging */
}

void process_manager_initialize(void) {
    for (int i = 0; i < MAX_PROCS; i++) {
        proctab[i].pid = -1;
        proctab[i].state = PR_TERMINATED;
        proctab[i].entry = NULL;
        proctab[i].stack_base = NULL;
        proctab[i].esp = NULL;
        proctab[i].mem = NULL;
        proctab[i].memsz = 0;
        proctab[i].sleep_ticks = 0;
        proctab[i].wait_event = -1;
        proctab[i].priority = 1;
        proctab[i].dyn_priority = 1;
    }

    serial_puts("Process manager initialized.\n");
}

/* -------------------------------------------------- */
/* Process Creation                                   */
/* -------------------------------------------------- */

int32_t process_create(void (*func)(void)) {
    int available_pid;

    for (available_pid = 0; available_pid < MAX_PROCS; available_pid++) {
        if (proctab[available_pid].state == PR_TERMINATED)
            break;
    }

    if (available_pid == MAX_PROCS)
        return -1;

    /* Simple process setup - no stack switching needed */
    proctab[available_pid].pid = available_pid;
    proctab[available_pid].state = PR_READY;
    proctab[available_pid].entry = func;
    proctab[available_pid].stack_base = NULL;
    proctab[available_pid].esp = NULL;
    proctab[available_pid].mem = NULL;
    proctab[available_pid].memsz = 0;
    proctab[available_pid].priority = 1;
    proctab[available_pid].dyn_priority = 1;

    serial_puts("Process created with PID: ");
    serial_put_int(available_pid);
    serial_puts("\n");

    return available_pid;
}

/* -------------------------------------------------- */
/* Process Exit                                       */
/* -------------------------------------------------- */

void process_terminate(void) {
    if (currpid) {
        proctab[currpid->pid].state = PR_TERMINATED;
        currpid = NULL;
        current_pid = -1;
    }
}

/* -------------------------------------------------- */
/* Process List                                       */
/* -------------------------------------------------- */

void process_list_display(void) {
    serial_puts("PID\tSTATE\n");
    serial_puts("----------------\n");

    for (int i = 0; i < MAX_PROCS; i++) {
        if (proctab[i].state != PR_TERMINATED) {
            serial_put_int(i);
            serial_puts("\t");

            switch (proctab[i].state) {
                case PR_CURRENT: serial_puts("RUNNING"); break;
                case PR_READY:   serial_puts("READY");   break;
                case PR_SLEEP:   serial_puts("SLEEP");   break;
                case PR_WAIT:    serial_puts("WAIT");    break;
                default:         serial_puts("UNKNOWN"); break;
            }

            serial_puts("\n");
        }
    }
    serial_puts("\n");
}

