#pragma once
#include "kernel.h"
/* Simple allocation wrappers with optional debug tracking */

void* kalloc(size_t size);
void  kfree(void* ptr);
void* kalloc_aligned(size_t size, size_t align);

/* Debug / stats */
typedef struct kmem_stats {
    u64 alloc_count;
    u64 free_count;
    u64 bytes_live;
    u64 bytes_peak;
    u64 leaks; /* live blocks at shutdown */
} kmem_stats_t;

const kmem_stats_t* kmem_get_stats(void);
void kmem_dump_leaks(void);

/* Enable/disable magic checking */
void kmem_set_debug(int enabled);
