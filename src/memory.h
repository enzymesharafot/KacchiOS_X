/* memory.h - Memory Manager Interface */
#ifndef MEMORY_H
#define MEMORY_H

#include "types.h"

/* Memory block header for heap allocation */
typedef struct mem_block {
    size_t size;              /* Size of the block (excluding header) */
    int is_free;              /* 1 if free, 0 if allocated */
    struct mem_block *next;   /* Next block in the linked list */
} mem_block_t;

/* Memory manager statistics */
typedef struct {
    uint32_t total_heap_size;
    uint32_t used_heap_size;
    uint32_t free_heap_size;
    uint32_t num_allocations;
    uint32_t num_deallocations;
} mem_stats_t;

/* Memory manager initialization */
void mem_init(void *heap_start, size_t heap_size);

/* Stack allocation (simple bump allocator) */
void *stack_alloc(size_t size);

/* Heap allocation (malloc-like) */
void *heap_alloc(size_t size);

/* Heap deallocation (free-like) */
void heap_free(void *ptr);

/* XINU-style aliases */
void *mem_alloc(size_t size);
void mem_free(void *ptr);

/* Get memory statistics */
void mem_get_stats(mem_stats_t *stats);

/* Print memory statistics (for debugging) */
void mem_print_stats(void);

#endif
