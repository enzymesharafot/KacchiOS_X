/* kernel.c - Main kernel with null process */
#include "types.h"
#include "serial.h"
#include "string.h"
#include "src/memory.h"
#include "src/process.h"
#include "src/scheduler.h"

#define MAX_INPUT 128

/* Memory region for our memory manager (128 KB) */
#define MEMORY_SIZE (128 * 1024)
static uint8_t kernel_memory[MEMORY_SIZE] __attribute__((aligned(16)));

/* Sample process functions */
void process_a(void) {
    serial_puts("[Process A] Starting...\n");
    for (int i = 0; i < 3; i++) {
        serial_puts("[Process A] Running iteration ");
        serial_put_uint(i + 1);
        serial_puts("\n");
        
        /* Simulate some work */
        for (volatile int j = 0; j < 1000000; j++);
        
        /* Yield to other processes */
        sched_yield();
    }
    serial_puts("[Process A] Completed!\n");
    proc_terminate(proc_get_pid());
}

void process_b(void) {
    serial_puts("[Process B] Starting...\n");
    for (int i = 0; i < 3; i++) {
        serial_puts("[Process B] Running iteration ");
        serial_put_uint(i + 1);
        serial_puts("\n");
        
        /* Simulate some work */
        for (volatile int j = 0; j < 1000000; j++);
        
        /* Yield to other processes */
        sched_yield();
    }
    serial_puts("[Process B] Completed!\n");
    proc_terminate(proc_get_pid());
}

void process_c(void) {
    serial_puts("[Process C] Starting (Low Priority)...\n");
    for (int i = 0; i < 2; i++) {
        serial_puts("[Process C] Running iteration ");
        serial_put_uint(i + 1);
        serial_puts("\n");
        
        /* Simulate some work */
        for (volatile int j = 0; j < 1000000; j++);
        
        /* Yield to other processes */
        sched_yield();
    }
    serial_puts("[Process C] Completed!\n");
    proc_terminate(proc_get_pid());
}

/* Test memory allocation */
void test_memory(void) {
    serial_puts("\n=== Testing Memory Manager ===\n");
    
    /* Test heap allocation */
    void *ptr1 = heap_alloc(100);
    serial_puts("Allocated 100 bytes at ");
    serial_put_hex((uint32_t)ptr1);
    serial_puts("\n");
    
    void *ptr2 = heap_alloc(200);
    serial_puts("Allocated 200 bytes at ");
    serial_put_hex((uint32_t)ptr2);
    serial_puts("\n");
    
    void *ptr3 = heap_alloc(50);
    serial_puts("Allocated 50 bytes at ");
    serial_put_hex((uint32_t)ptr3);
    serial_puts("\n");
    
    /* Free some memory */
    heap_free(ptr2);
    serial_puts("Freed 200 bytes\n");
    
    /* Allocate again (should reuse freed space) */
    void *ptr4 = heap_alloc(150);
    serial_puts("Allocated 150 bytes at ");
    serial_put_hex((uint32_t)ptr4);
    serial_puts("\n");
    
    /* Show statistics */
    mem_print_stats();
}

/* Demo the OS features */
void demo_os(void) {
    serial_puts("\n=== kacchiOS Demo ===\n\n");
    
    /* Test memory manager */
    test_memory();
    
    /* Create some processes */
    serial_puts("\n=== Creating Processes ===\n");
    int pid_a = proc_create("Process-A", process_a, PRIO_NORMAL, 4096);
    int pid_b = proc_create("Process-B", process_b, PRIO_NORMAL, 4096);
    int pid_c = proc_create("Process-C", process_c, PRIO_LOW, 4096);
    
    if (pid_a > 0 && pid_b > 0 && pid_c > 0) {
        /* Show process table */
        proc_print_table();
        
        /* Add processes to scheduler */
        serial_puts("\n=== Starting Scheduler ===\n");
        pcb_t *pcb_a = proc_get(pid_a);
        pcb_t *pcb_b = proc_get(pid_b);
        pcb_t *pcb_c = proc_get(pid_c);
        
        if (pcb_a) sched_add_ready(pcb_a);
        if (pcb_b) sched_add_ready(pcb_b);
        if (pcb_c) sched_add_ready(pcb_c);
        
        /* Run scheduler a few times */
        for (int i = 0; i < 10; i++) {
            sched_schedule();
            
            /* Simulate timer tick */
            for (int j = 0; j < 5; j++) {
                sched_tick();
            }
        }
        
        /* Show final statistics */
        serial_puts("\n=== Final Statistics ===\n");
        proc_print_table();
        sched_print_stats();
        mem_print_stats();
    }
}

void kmain(void) {
    char input[MAX_INPUT];
    int pos = 0;
    
    /* Initialize hardware */
    serial_init();
    
    /* Print welcome message */
    serial_puts("\n");
    serial_puts("========================================\n");
    serial_puts("    kacchiOS - Minimal Baremetal OS\n");
    serial_puts("========================================\n");
    serial_puts("Hello from kacchiOS!\n");
    serial_puts("Enhanced with Memory, Process, and Scheduler!\n\n");
    
    /* Initialize OS components */
    serial_puts("Initializing OS components...\n");
    mem_init(kernel_memory, MEMORY_SIZE);
    proc_init();
    sched_init(SCHED_PRIORITY_RR);
    serial_puts("All components initialized successfully!\n");
    
    /* Main loop - interactive shell */
    while (1) {
        serial_puts("\nkacchiOS> ");
        pos = 0;
        
        /* Read input line */
        while (1) {
            char c = serial_getc();
            
            /* Handle Enter key */
            if (c == '\r' || c == '\n') {
                input[pos] = '\0';
                serial_puts("\n");
                break;
            }
            /* Handle Backspace */
            else if ((c == '\b' || c == 0x7F) && pos > 0) {
                pos--;
                serial_puts("\b \b");  /* Erase character on screen */
            }
            /* Handle normal characters */
            else if (c >= 32 && c < 127 && pos < MAX_INPUT - 1) {
                input[pos++] = c;
                serial_putc(c);  /* Echo character */
            }
        }
        
        /* Process commands */
        if (pos > 0) {
            if (strcmp(input, "help") == 0) {
                serial_puts("Available commands:\n");
                serial_puts("  help     - Show this help message\n");
                serial_puts("  demo     - Run OS feature demo\n");
                serial_puts("  mem      - Show memory statistics\n");
                serial_puts("  ps       - Show process table\n");
                serial_puts("  sched    - Show scheduler statistics\n");
                serial_puts("  clear    - Clear screen\n");
                serial_puts("  about    - About kacchiOS\n");
            }
            else if (strcmp(input, "demo") == 0) {
                demo_os();
            }
            else if (strcmp(input, "mem") == 0) {
                mem_print_stats();
            }
            else if (strcmp(input, "ps") == 0) {
                proc_print_table();
            }
            else if (strcmp(input, "sched") == 0) {
                sched_print_stats();
            }
            else if (strcmp(input, "clear") == 0) {
                for (int i = 0; i < 50; i++) {
                    serial_puts("\n");
                }
            }
            else if (strcmp(input, "about") == 0) {
                serial_puts("\nkacchiOS - Educational Bare-metal OS\n");
                serial_puts("Version: 2.0 (with Memory, Process, Scheduler)\n");
                serial_puts("Features:\n");
                serial_puts("  - Memory Manager (Stack + Heap allocation)\n");
                serial_puts("  - Process Manager (PCB, state transitions)\n");
                serial_puts("  - Scheduler (Priority-based Round-Robin)\n");
                serial_puts("  - Serial I/O driver\n");
            }
            else {
                serial_puts("Unknown command: ");
                serial_puts(input);
                serial_puts("\nType 'help' for available commands.\n");
            }
        }
    }
    
    /* Should never reach here */
    for (;;) {
        __asm__ volatile ("hlt");
    }
}