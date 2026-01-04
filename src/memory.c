/* memory.c - Memory Manager Implementation */
#include "memory.h"
#include "string.h"
#include "serial.h"

/* Global memory management structures */
static uint8_t *stack_ptr = NULL;
static uint8_t *stack_end = NULL;
static mem_block_t *heap_head = NULL;
static mem_stats_t mem_stats;

/* Initialize the memory manager */
void mem_init(void *heap_start, size_t heap_size) {
    /* Clear statistics */
    mem_stats.total_heap_size = 0;
    mem_stats.used_heap_size = 0;
    mem_stats.free_heap_size = 0;
    mem_stats.num_allocations = 0;
    mem_stats.num_deallocations = 0;
    
    if (heap_start == NULL || heap_size == 0) {
        serial_puts("ERROR: Invalid heap parameters\n");
        return;
    }
    
    /* Initialize stack allocator (first half of memory) */
    size_t stack_size = heap_size / 2;
    stack_ptr = (uint8_t *)heap_start;
    stack_end = stack_ptr + stack_size;
    
    /* Initialize heap allocator (second half of memory) */
    uint8_t *heap_area = stack_end;
    size_t heap_actual_size = heap_size - stack_size;
    
    /* Create initial free block */
    heap_head = (mem_block_t *)heap_area;
    heap_head->size = heap_actual_size - sizeof(mem_block_t);
    heap_head->is_free = 1;
    heap_head->next = NULL;
    
    /* Update statistics */
    mem_stats.total_heap_size = heap_actual_size;
    mem_stats.free_heap_size = heap_head->size;
    
    serial_puts("Memory Manager initialized:\n");
    serial_puts("  Stack size: ");
    serial_put_uint(stack_size);
    serial_puts(" bytes\n");
    serial_puts("  Heap size: ");
    serial_put_uint(heap_actual_size);
    serial_puts(" bytes\n");
}

/* Stack allocation - simple bump allocator */
void *stack_alloc(size_t size) {
    if (stack_ptr == NULL) {
        serial_puts("ERROR: Memory manager not initialized\n");
        return NULL;
    }
    
    /* Align to 4-byte boundary */
    size = (size + 3) & ~3;
    
    if (stack_ptr + size > stack_end) {
        serial_puts("ERROR: Stack allocation failed - out of memory\n");
        return NULL;
    }
    
    void *ptr = stack_ptr;
    stack_ptr += size;
    
    /* Zero out the allocated memory */
    memset(ptr, 0, size);
    
    return ptr;
}

/* Find a free block that fits the requested size (first-fit algorithm) */
static mem_block_t *find_free_block(size_t size) {
    mem_block_t *current = heap_head;
    
    while (current != NULL) {
        if (current->is_free && current->size >= size) {
            return current;
        }
        current = current->next;
    }
    
    return NULL;
}

/* Split a block if it's large enough */
static void split_block(mem_block_t *block, size_t size) {
    size_t remaining = block->size - size - sizeof(mem_block_t);
    
    /* Only split if remaining space is useful (at least 16 bytes) */
    if (remaining > 16) {
        mem_block_t *new_block = (mem_block_t *)((uint8_t *)block + sizeof(mem_block_t) + size);
        new_block->size = remaining;
        new_block->is_free = 1;
        new_block->next = block->next;
        
        block->size = size;
        block->next = new_block;
    }
}

/* Heap allocation - first-fit algorithm */
void *heap_alloc(size_t size) {
    if (heap_head == NULL) {
        serial_puts("ERROR: Memory manager not initialized\n");
        return NULL;
    }
    
    if (size == 0) {
        return NULL;
    }
    
    /* Align to 4-byte boundary */
    size = (size + 3) & ~3;
    
    /* Find a free block */
    mem_block_t *block = find_free_block(size);
    if (block == NULL) {
        serial_puts("ERROR: Heap allocation failed - no suitable block found\n");
        return NULL;
    }
    
    /* Split the block if it's too large */
    split_block(block, size);
    
    /* Mark block as allocated */
    block->is_free = 0;
    
    /* Update statistics */
    mem_stats.used_heap_size += block->size;
    mem_stats.free_heap_size -= block->size;
    mem_stats.num_allocations++;
    
    /* Return pointer to the usable memory (after header) */
    void *ptr = (uint8_t *)block + sizeof(mem_block_t);
    
    /* Zero out the allocated memory */
    memset(ptr, 0, size);
    
    return ptr;
}

/* Merge adjacent free blocks */
static void merge_free_blocks(void) {
    mem_block_t *current = heap_head;
    
    while (current != NULL && current->next != NULL) {
        if (current->is_free && current->next->is_free) {
            /* Merge current with next */
            current->size += sizeof(mem_block_t) + current->next->size;
            current->next = current->next->next;
            /* Don't advance, check if we can merge with the new next block */
        } else {
            current = current->next;
        }
    }
}

/* Heap deallocation */
void heap_free(void *ptr) {
    if (ptr == NULL) {
        return;
    }
    
    /* Get the block header */
    mem_block_t *block = (mem_block_t *)((uint8_t *)ptr - sizeof(mem_block_t));
    
    /* Validate the block (basic sanity check) */
    if (block->is_free) {
        serial_puts("WARNING: Double free detected\n");
        return;
    }
    
    /* Mark block as free */
    block->is_free = 1;
    
    /* Update statistics */
    mem_stats.used_heap_size -= block->size;
    mem_stats.free_heap_size += block->size;
    mem_stats.num_deallocations++;
    
    /* Merge adjacent free blocks */
    merge_free_blocks();
}

/* Get memory statistics */
void mem_get_stats(mem_stats_t *stats) {
    if (stats == NULL) {
        return;
    }
    
    stats->total_heap_size = mem_stats.total_heap_size;
    stats->used_heap_size = mem_stats.used_heap_size;
    stats->free_heap_size = mem_stats.free_heap_size;
    stats->num_allocations = mem_stats.num_allocations;
    stats->num_deallocations = mem_stats.num_deallocations;
}

/* Print memory statistics */
void mem_print_stats(void) {
    serial_puts("\n=== Memory Statistics ===\n");
    serial_puts("Total heap size: ");
    serial_put_uint(mem_stats.total_heap_size);
    serial_puts(" bytes\n");
    serial_puts("Used heap size: ");
    serial_put_uint(mem_stats.used_heap_size);
    serial_puts(" bytes\n");
    serial_puts("Free heap size: ");
    serial_put_uint(mem_stats.free_heap_size);
    serial_puts(" bytes\n");
    serial_puts("Allocations: ");
    serial_put_uint(mem_stats.num_allocations);
    serial_puts("\n");
    serial_puts("Deallocations: ");
    serial_put_uint(mem_stats.num_deallocations);
    serial_puts("\n");
}

/* XINU-style aliases */
void *mem_alloc(size_t size) {
    return heap_alloc(size);
}

void mem_free(void *ptr) {
    heap_free(ptr);
}
