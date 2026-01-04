/* scheduler.c - Scheduler Implementation (XINU Style) */
#include "scheduler.h"

/*
 * In XINU style, the scheduler is integrated into the process manager.
 * All scheduling functions (resched, yield, sleep, wait, wakeup) 
 * are implemented in process.c.
 * 
 * This file is kept for compatibility but contains no code.
 * See process.c for the actual scheduler implementation.
 */

    sched_stats.total_context_switches = 0;
    sched_stats.total_scheduler_calls = 0;
    sched_stats.idle_time = 0;
    
    serial_puts("Scheduler initialized (policy: ");
    switch (policy) {
        case SCHED_ROUND_ROBIN:
            serial_puts("Round-Robin");
            break;
        case SCHED_PRIORITY:
            serial_puts("Priority");
            break;
        case SCHED_PRIORITY_RR:
            serial_puts("Priority with Round-Robin");
            break;
    }
    serial_puts(", quantum: ");
    serial_put_uint(default_quantum);
    serial_puts(")\n");
}

/* Add a process to the ready queue */
void sched_add_ready(pcb_t *pcb) {
    if (pcb == NULL) {
        return;
    }
    
    /* Add to appropriate priority queue */
    queue_enqueue(&ready_queues[pcb->priority], pcb->pid);
    
    /* Update process state */
    pcb->state = PROC_READY;
}

/* Remove a process from the ready queue */
void sched_remove_ready(uint32_t pid) {
    pcb_t *pcb = proc_get(pid);
    if (pcb == NULL) {
        return;
    }
    
    /* Remove from appropriate priority queue */
    queue_remove(&ready_queues[pcb->priority], pid);
}

/* Select next process based on scheduling policy */
static pcb_t *select_next_process(void) {
    uint32_t next_pid = 0;
    
    switch (current_policy) {
        case SCHED_ROUND_ROBIN:
            /* Simple round-robin: check all queues in order */
            for (int i = 0; i < 3; i++) {
                if (!queue_is_empty(&ready_queues[i])) {
                    next_pid = queue_dequeue(&ready_queues[i]);
                    break;
                }
            }
            break;
            
        case SCHED_PRIORITY:
        case SCHED_PRIORITY_RR:
            /* Priority-based: check high priority first, then normal, then low */
            for (int i = 0; i < 3; i++) {
                if (!queue_is_empty(&ready_queues[i])) {
                    next_pid = queue_dequeue(&ready_queues[i]);
                    break;
                }
            }
            break;
    }
    
    if (next_pid == 0) {
        return NULL;
    }
    
    return proc_get(next_pid);
}

/* Context switch (simplified version - saves/restores minimal context) */
void sched_switch_context(pcb_t *old_proc, pcb_t *new_proc) {
    if (new_proc == NULL) {
        return;
    }
    
    /* Update statistics */
    sched_stats.total_context_switches++;
    
    /* Save old process state if it exists */
    if (old_proc != NULL) {
        /* In a real OS, we'd save registers here using inline assembly */
        /* For this educational OS, we just update the state */
        if (old_proc->state == PROC_CURRENT) {
            old_proc->state = PROC_READY;
            sched_add_ready(old_proc);
        }
    }
    
    /* Load new process state */
    new_proc->state = PROC_CURRENT;
    proc_set_current(new_proc->pid);
    new_proc->time_quantum = default_quantum;
    
    /* In a real OS, we'd restore registers and jump to the new process here */
    /* For this educational OS, we simulate it */
    
    serial_puts("[SCHED] Switched to process '");
    serial_puts(new_proc->name);
    serial_puts("' (PID ");
    serial_put_uint(new_proc->pid);
    serial_puts(")\n");
}

/* Schedule the next process to run */
void sched_schedule(void) {
    sched_stats.total_scheduler_calls++;
    
    pcb_t *current = proc_current();
    pcb_t *next = select_next_process();
    
    if (next == NULL) {
        /* No process ready to run - idle */
        sched_stats.idle_time++;
        serial_puts("[SCHED] No ready process, idle...\n");
        return;
    }
    
    /* If next is the same as current, just continue */
    if (current != NULL && current->pid == next->pid) {
        return;
    }
    
    /* Perform context switch */
    sched_switch_context(current, next);
}

/* Yield CPU to another process */
void sched_yield(void) {
    pcb_t *current = proc_current();
    
    if (current != NULL) {
        serial_puts("[SCHED] Process ");
        serial_put_uint(current->pid);
        serial_puts(" yielding CPU\n");
        
        /* Move current process back to ready queue */
        current->state = PROC_READY;
        sched_add_ready(current);
    }
    
    /* Schedule next process */
    sched_schedule();
}

/* Update time quantum for current process (called on timer tick) */
void sched_tick(void) {
    pcb_t *current = proc_current();
    
    if (current == NULL) {
        return;
    }
    
    /* Decrement time quantum */
    if (current->time_quantum > 0) {
        current->time_quantum--;
        current->total_runtime++;
    }
    
    /* If quantum expired, schedule next process */
    if (current->time_quantum == 0) {
        serial_puts("[SCHED] Time quantum expired for PID ");
        serial_put_uint(current->pid);
        serial_puts("\n");
        sched_yield();
    }
}

/* Get scheduler statistics */
void sched_get_stats(sched_stats_t *stats) {
    if (stats == NULL) {
        return;
    }
    
    stats->total_context_switches = sched_stats.total_context_switches;
    stats->total_scheduler_calls = sched_stats.total_scheduler_calls;
    stats->idle_time = sched_stats.idle_time;
}

/* Print scheduler statistics */
void sched_print_stats(void) {
    serial_puts("\n=== Scheduler Statistics ===\n");
    serial_puts("Policy: ");
    switch (current_policy) {
        case SCHED_ROUND_ROBIN:
            serial_puts("Round-Robin\n");
            break;
        case SCHED_PRIORITY:
            serial_puts("Priority\n");
            break;
        case SCHED_PRIORITY_RR:
            serial_puts("Priority with Round-Robin\n");
            break;
    }
    serial_puts("Time quantum: ");
    serial_put_uint(default_quantum);
    serial_puts("\n");
    serial_puts("Total context switches: ");
    serial_put_uint(sched_stats.total_context_switches);
    serial_puts("\n");
    serial_puts("Total scheduler calls: ");
    serial_put_uint(sched_stats.total_scheduler_calls);
    serial_puts("\n");
    serial_puts("Idle time: ");
    serial_put_uint(sched_stats.idle_time);
    serial_puts("\n");
}

/* Set time quantum for round-robin scheduling */
void sched_set_quantum(uint32_t quantum) {
    if (quantum > 0) {
        default_quantum = quantum;
        serial_puts("Time quantum set to ");
        serial_put_uint(quantum);
        serial_puts("\n");
    }
}
