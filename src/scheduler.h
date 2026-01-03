/* scheduler.h - Scheduler Interface */
#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "../types.h"
#include "process.h"

/* Scheduling policy */
typedef enum {
    SCHED_ROUND_ROBIN,      /* Round-robin scheduling */
    SCHED_PRIORITY,         /* Priority-based scheduling */
    SCHED_PRIORITY_RR       /* Priority with round-robin within same priority */
} sched_policy_t;

/* Scheduler statistics */
typedef struct {
    uint32_t total_context_switches;
    uint32_t total_scheduler_calls;
    uint32_t idle_time;
} sched_stats_t;

/* Initialize the scheduler */
void sched_init(sched_policy_t policy);

/* Schedule the next process to run */
void sched_schedule(void);

/* Add a process to the ready queue */
void sched_add_ready(pcb_t *pcb);

/* Remove a process from the ready queue */
void sched_remove_ready(uint32_t pid);

/* Yield CPU to another process */
void sched_yield(void);

/* Update time quantum for current process */
void sched_tick(void);

/* Context switch to another process */
void sched_switch_context(pcb_t *old_proc, pcb_t *new_proc);

/* Get scheduler statistics */
void sched_get_stats(sched_stats_t *stats);

/* Print scheduler statistics */
void sched_print_stats(void);

/* Set time quantum (for round-robin) */
void sched_set_quantum(uint32_t quantum);

#endif
