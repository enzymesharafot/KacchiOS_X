#include "memory.h"
#include "serial.h"

#define HEAP_SIZE 64*1024  // 64 KB heap size

typedef struct mem_block
{
    size_t size;
    int free;
    struct mem_block *next;
} mem_block_t;

static uint8_t heap[HEAP_SIZE];
static mem_block_t *free_list = NULL;

// Initialize the memory manager
void memory_manager_initialize(void){
    free_list = (mem_block_t*)heap;
    free_list->size = HEAP_SIZE - sizeof(mem_block_t);
    free_list->free = 1;
    free_list->next = NULL;

    serial_puts("Memory manager initialized.\n");
}

// Allocate memory
void *memory_allocate(size_t size){
    mem_block_t *current_block = free_list;
    size = (size + 3) & ~3; // Align size to 4 bytes
    while (current_block)
    {
        if(current_block->free && current_block->size >= size){
            if (current_block->size > size + sizeof(mem_block_t)){
                mem_block_t* new_block = (mem_block_t*)((uint8_t*)current_block + sizeof(mem_block_t) + size);
                new_block->size = current_block->size - size - sizeof(mem_block_t);
                new_block->free = 1;
                new_block->next = current_block->next;

                current_block->next = new_block;
                current_block->size = size;
            }
            current_block->free = 0;
            return (uint8_t*)current_block + sizeof(mem_block_t);
        }
        current_block = current_block->next;
    }
    return NULL;
}


// Free allocated memory
void memory_deallocate(void *ptr){
    if (!ptr) return;

    mem_block_t* freed_block = (mem_block_t*)((uint8_t*)ptr - sizeof(mem_block_t));
    freed_block->free = 1;

    mem_block_t* current_block = free_list;
    while(current_block && current_block->next){
        if(current_block->free && current_block->next->free){
            current_block->size += sizeof(mem_block_t) + current_block->next->size;
            current_block->next = current_block->next->next;
        }else{
            current_block = current_block->next;
        }
    }
}

