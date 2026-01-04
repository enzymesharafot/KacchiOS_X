/* kernel.c - Main kernel (XINU Style) */
#include "types.h"
#include "serial.h"
#include "string.h"
#include "memory.h"
#include "process.h"

#define MAX_INPUT 128

/* Memory region for our memory manager (128 KB) */
#define MEMORY_SIZE (128 * 1024)
static uint8_t kernel_memory[MEMORY_SIZE] __attribute__((aligned(16)));

/* Null process - idle process */
void null_process(void) {
    serial_puts("[NULL] Null process running (idle)\n");
    while (1) {
        /* Idle loop */
        for (volatile int i = 0; i < 100000; i++);
        yield();
    }
}

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
        yield();
    }
    serial_puts("[Process A] Completed!\n");
    proc_exit();
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
        yield();
    }
    serial_puts("[Process B] Completed!\n");
    proc_exit();
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
        yield();
    }
    serial_puts("[Process C] Completed!\n");
    proc_exit();
}

/* Test memory allocation */
void test_memory(void) {
    serial_puts("\n=== Testing Memory Manager ===\n");
    
    /* Test heap allocation */
    void *ptr1 = mem_alloc(100);
    serial_puts("Allocated 100 bytes at ");
    serial_put_hex((uint32_t)ptr1);
    serial_puts("\n");
    
    void *ptr2 = mem_alloc(200);
    serial_puts("Allocated 200 bytes at ");
    serial_put_hex((uint32_t)ptr2);
    serial_puts("\n");
    
    void *ptr3 = mem_alloc(50);
    serial_puts("Allocated 50 bytes at ");
    serial_put_hex((uint32_t)ptr3);
    serial_puts("\n");
    
    /* Free some memory */
    mem_free(ptr2);
    serial_puts("Freed 200 bytes\n");
    
    /* Allocate again (should reuse freed space) */
    void *ptr4 = mem_alloc(150);
    serial_puts("Allocated 150 bytes at ");
    serial_put_hex((uint32_t)ptr4);
    serial_puts("\n");
    
    /* Show statistics */
    mem_print_stats();
}

/* Demo the OS features - XINU Style */
void demo_os(void) {
    serial_puts("\n=== kacchiOS Demo (XINU Style) ===\n\n");
    
    /* Test memory manager */
    test_memory();
    
    /* Create some processes */
    serial_puts("\n=== Creating Processes ===\n");
    
    /* Create processes - XINU style (simplified) */
    proc_create(process_a);
    proc_create(process_b);
    proc_create(process_c);
    
    /* Show process table */
    serial_puts("\n");
    proc_list();
    
    serial_puts("\nProcesses created. Type 'run' to start scheduling.\n");
}

void kmain(void) {
    char input[MAX_INPUT];
    int pos = 0;
    
    /* Initialize hardware */
    serial_init();
    
    /* Print welcome message */
    serial_puts("\n");
    serial_puts("========================================\n");
    serial_puts("    kacchiOS - XINU Style OS\n");
    serial_puts("========================================\n");
    serial_puts("Hello from kacchiOS!\n");
    serial_puts("XINU-style Process Manager & Scheduler\n\n");
    
    /* Initialize OS components */
    serial_puts("Initializing OS components...\n");
    mem_init(kernel_memory, MEMORY_SIZE);
    proc_init();
    serial_puts("All components initialized successfully!\n");
    
    /* Create null process (PID 0) */
    proc_create(null_process);
    
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
                serial_puts("  demo     - Create demo processes\n");
                serial_puts("  run      - Start process scheduling\n");
                serial_puts("  mem      - Show memory statistics\n");
                serial_puts("  ps       - Show process list\n");
                serial_puts("  clear    - Clear screen\n");
                serial_puts("  about    - About kacchiOS\n");
            }
            else if (strcmp(input, "demo") == 0) {
                demo_os();
            }
            else if (strcmp(input, "run") == 0) {
                serial_puts("Starting process scheduler...\n");
                proc_run();  /* This will not return */
            }
            else if (strcmp(input, "mem") == 0) {
                mem_print_stats();
            }
            else if (strcmp(input, "ps") == 0) {
                proc_list();
            }
            else if (strcmp(input, "clear") == 0) {
                for (int i = 0; i < 50; i++) {
                    serial_puts("\n");
                }
            }
            else if (strcmp(input, "about") == 0) {
                serial_puts("\nkacchiOS - Educational Bare-metal OS\n");
                serial_puts("Version: 3.0 (XINU Style)\n");
                serial_puts("Features:\n");
                serial_puts("  - Memory Manager (Heap allocation)\n");
                serial_puts("  - Process Manager (XINU-style PCB)\n");
                serial_puts("  - Scheduler (Priority + Aging)\n");
                serial_puts("  - Context Switching\n");
                serial_puts("  - Sleep/Wait/Wakeup\n");
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