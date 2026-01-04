/* kernel.c - Main kernel */
#include "types.h"
#include "serial.h"
#include "string.h"
#include "memory.h"
#include "process.h"

#define MAX_INPUT 128

/* Null process - idle process */
void null_process(void) {
    serial_puts("[NULL] Null process running (idle)\n");
    while (1) {
        /* Idle loop */
        for (volatile int i = 0; i < 100000; i++);
        process_yield_cpu();
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
        process_yield_cpu();
    }
    serial_puts("[Process A] Completed!\n");
    process_terminate();
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
        process_yield_cpu();
    }
    serial_puts("[Process B] Completed!\n");
    process_terminate();
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
        process_yield_cpu();
    }
    serial_puts("[Process C] Completed!\n");
    process_terminate();
}

/* Test memory allocation */
void test_memory(void) {
    serial_puts("\n=== Testing Memory Manager ===\n");
    
    /* Test heap allocation */
    void *memory_block_1 = memory_allocate(100);
    serial_puts("Allocated 100 bytes at ");
    serial_put_hex((uint32_t)memory_block_1);
    serial_puts("\n");
    
    void *memory_block_2 = memory_allocate(200);
    serial_puts("Allocated 200 bytes at ");
    serial_put_hex((uint32_t)memory_block_2);
    serial_puts("\n");
    
    void *memory_block_3 = memory_allocate(50);
    serial_puts("Allocated 50 bytes at ");
    serial_put_hex((uint32_t)memory_block_3);
    serial_puts("\n");
    
    /* Free some memory */
    memory_deallocate(memory_block_2);
    serial_puts("Freed 200 bytes\n");
    
    /* Allocate again (should reuse freed space) */
    void *memory_block_4 = memory_allocate(150);
    serial_puts("Allocated 150 bytes at ");
    serial_put_hex((uint32_t)memory_block_4);
    serial_puts("\n");
}

/* Demo the OS features - XINU Style */
void demo_os(void) {
    serial_puts("\n=== kacchiOS Demo (XINU Style) ===\n\n");
    
    /* Test memory manager */
    test_memory();
    
    /* Create some processes */
    serial_puts("\n=== Creating Processes ===\n");
    
    /* Create processes - XINU style (simplified) */
    process_create(process_a);
    process_create(process_b);
    process_create(process_c);
    
    /* Show process table */
    serial_puts("\n");
    process_list_display();
    
    serial_puts("\nProcesses created. Type 'run' to start scheduling.\n");
}

void kmain(void) {
    char user_input[MAX_INPUT];
    int input_position = 0;
    
    /* Initialize hardware */
    serial_init();
    
    /* Print welcome message */
    serial_puts("\n");
    serial_puts("========================================\n");
    serial_puts("              KacchiOS_X                \n");
    serial_puts("========================================\n");
    serial_puts("         Hello from kacchiOS!           \n");
    
    /* Initialize OS components */
    serial_puts("Initializing OS components...\n");
    memory_manager_initialize();
    process_manager_initialize();
    serial_puts("All components initialized successfully!\n");
    
    /* Create null process (PID 0) */
    process_create(null_process);
    
    /* Main loop - interactive shell */
    while (1) {
        serial_puts("\nX_Kacchi> ");
        input_position = 0;
        
        /* Read input line */
        while (1) {
            char input_char = serial_getc();
            
            /* Handle Enter key */
            if (input_char == '\r' || input_char == '\n') {
                user_input[input_position] = '\0';
                serial_puts("\n");
                break;
            }
            /* Handle Backspace */
            else if ((input_char == '\b' || input_char == 0x7F) && input_position > 0) {
                input_position--;
                serial_puts("\b \b");  /* Erase character on screen */
            }
            /* Handle normal characters */
            else if (input_char >= 32 && input_char < 127 && input_position < MAX_INPUT - 1) {
                user_input[input_position++] = input_char;
                serial_putc(input_char);  /* Echo character */
            }
        }
        
        /* Process commands */
        if (input_position > 0) {
            if (strcmp(user_input, "help") == 0) {
                serial_puts("Available commands:\n");
                serial_puts("  help     - Show this help message\n");
                serial_puts("  demo     - Create demo processes\n");
                serial_puts("  run      - Start process scheduling\n");
                serial_puts("  mem      - Show memory statistics\n");
                serial_puts("  ps       - Show process list\n");
                serial_puts("  clear    - Clear screen\n");
                serial_puts("  about    - About kacchiOS\n");
            }
            else if (strcmp(user_input, "demo") == 0) {
                demo_os();
            }
            else if (strcmp(user_input, "run") == 0) {
                serial_puts("Starting process scheduler...\n");
                process_scheduler_start();  /* This will not return */
            }
            else if (strcmp(user_input, "mem") == 0) {
                serial_puts("Memory manager active (64KB heap)\n");
            }
            else if (strcmp(user_input, "ps") == 0) {
                process_list_display();
            }
            else if (strcmp(user_input, "clear") == 0) {
                for (int i = 0; i < 50; i++) {
                    serial_puts("\n");
                }
            }
            else if (strcmp(user_input, "about") == 0) {
                serial_puts("\nkacchiOS - Educational Bare-metal OS\n");
                serial_puts("Version: 3.0 (XINU Style)\n");
                serial_puts("Features:\n");
                serial_puts("  - Memory Manager (Heap allocation)\n");
                serial_puts("  - Process Manager \n");
                serial_puts("  - Scheduler (Priority + Aging)\n");
                serial_puts("  - Context Switching\n");
                serial_puts("  - Sleep/Wait/Wakeup\n");
            }
            else {
                serial_puts("Unknown command: ");
                serial_puts(user_input);
                serial_puts("\nType 'help' for available commands.\n");
            }
        }
    }
    
    /* Should never reach here */
    for (;;) {
        __asm__ volatile ("hlt");
    }
}