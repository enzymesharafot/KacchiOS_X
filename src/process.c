/* process.c - Process Manager Implementation */
#include "process.h"
#include "serial.h"
#include "memory.h"

#define PROC_STACK_SIZE 4096

static pcb_t proctab[MAX_PROCS];
static int32_t current_pid = -1;
pcb_t *currpid = NULL;

static int first_dispatch = 1;

extern void ctxsw(uint32_t **old, uint32_t **new);

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
/* Sleep & Wait Functions                             */
/* -------------------------------------------------- */

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
    /* Find first ready process (should be PID 0 - null_process) */
    int next_pid = -1;
    for (int i = 0; i < MAX_PROCS; i++) {
        if (proctab[i].state == PR_READY) {
            next_pid = i;
            break;
        }
    }
    
    if (next_pid == -1) {
        serial_puts("ERROR: No ready process to run!\n");
        while (1);
    }
    
    /* Set up initial process */
    proctab[next_pid].state = PR_CURRENT;
    current_pid = next_pid;
    currpid = &proctab[next_pid];
    
    /* First dispatch (bootstrap) */
    first_dispatch = 0;
    asm volatile(
        "movl %0, %%esp \n"
        "jmp  *%1      \n"
        :
        : "r"(proctab[next_pid].esp),
          "r"(proctab[next_pid].entry)
    );
    
    /* Should never reach here */
    while (1);
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

    uint32_t *process_stack = memory_allocate(PROC_STACK_SIZE);
    if (!process_stack) {
        serial_puts("Stack allocation failed.\n");
        return -1;
    }

    uint32_t *stack_pointer = (uint32_t *)((uint32_t)process_stack + PROC_STACK_SIZE);
    stack_pointer = (uint32_t *)((uint32_t)stack_pointer & ~0xF);   // align

    *--stack_pointer = (uint32_t)process_terminate;  // return address
    *--stack_pointer = (uint32_t)func;       // first EIP

    proctab[available_pid].pid = available_pid;
    proctab[available_pid].state = PR_READY;
    proctab[available_pid].entry = func;
    proctab[available_pid].stack_base = process_stack;
    proctab[available_pid].esp = stack_pointer;
    proctab[available_pid].mem = process_stack;
    proctab[available_pid].memsz = PROC_STACK_SIZE;
    proctab[available_pid].priority = 1;
    proctab[available_pid].dyn_priority = 1;

    serial_puts("Process created with PID: ");
    serial_put_int(available_pid);
    serial_puts("\n");

    return available_pid;
}

/* -------------------------------------------------- */
/* Scheduler Core                                     */
/* -------------------------------------------------- */

void scheduler_reschedule(void) {
    scheduler_update_aging();

    int previous_pid = current_pid;
    int next_pid = -1;
    int highest_priority = -1;

    /* Round-robin search */
    for (int i = 0; i < MAX_PROCS; i++) {
        if (proctab[i].state == PR_READY) {
            if (proctab[i].dyn_priority > highest_priority) {
                highest_priority = proctab[i].dyn_priority;
                next_pid = i;
            }
        }
    }

    /* No READY process → idle (PID 0) */
    if (next_pid == -1)
        next_pid = 0;

    /* Reset priority of running process */
    proctab[next_pid].dyn_priority = proctab[next_pid].priority;

    /* Already running */
    if (next_pid == previous_pid && previous_pid >= 0)
        return;

    /* Update states */
    if (previous_pid >= 0 && proctab[previous_pid].state == PR_CURRENT)
        proctab[previous_pid].state = PR_READY;

    proctab[next_pid].state = PR_CURRENT;
    current_pid = next_pid;
    currpid = &proctab[next_pid];

    /* First dispatch (bootstrap) */
    if (first_dispatch) {
        first_dispatch = 0;
        asm volatile(
            "movl %0, %%esp \n"
            "jmp  *%1      \n"
            :
            : "r"(proctab[next_pid].esp),
              "r"(proctab[next_pid].entry)
        );
        while (1);
    }

    /* Normal context switch */
    ctxsw(&proctab[previous_pid].esp, &proctab[next_pid].esp);
}

/* -------------------------------------------------- */
/* Yield                                              */
/* -------------------------------------------------- */

void process_yield_cpu(void) {
    if (currpid)
        currpid->state = PR_READY;
    scheduler_reschedule();
}

/* -------------------------------------------------- */
/* Process Exit                                       */
/* -------------------------------------------------- */

void process_terminate(void) {
    int process_pid = currpid->pid;

    proctab[process_pid].state = PR_TERMINATED;
    memory_deallocate(proctab[process_pid].mem);

    proctab[process_pid].mem = NULL;
    proctab[process_pid].esp = NULL;

    serial_puts("Process exited: ");
    serial_put_int(process_pid);
    serial_puts("\n");

    currpid = NULL;
    current_pid = -1;

    scheduler_reschedule();
    while (1);
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

/* -------------------------------------------------- */
/* Aging Update                                       */
/* -------------------------------------------------- */

void scheduler_update_aging(void) {
    for (int i = 0; i < MAX_PROCS; i++) {
        if (proctab[i].state == PR_READY) {
            proctab[i].dyn_priority++;
        }
    }
}
