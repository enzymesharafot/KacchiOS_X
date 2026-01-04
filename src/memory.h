/* memory.h - Memory Manager Interface */
#ifndef MEMORY_H
#define MEMORY_H

#include "types.h"

/* Memory manager initialization */
void memory_manager_initialize(void);

/* Memory allocation */
void *memory_allocate(size_t size);

/* Memory deallocation */
void memory_deallocate(void *ptr);

#endif
