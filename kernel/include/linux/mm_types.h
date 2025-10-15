#ifndef _LINUX_MM_TYPES_H
#define _LINUX_MM_TYPES_H

#include <linux/list.h>
#include <linux/atomic.h>
#include <linux/spinlock.h>

// From page-flags.h
#define PG_locked 0

/*
 * A page descriptor.
 * This is a simplified version.
 */
typedef struct page {
    unsigned long flags;        /* Atomic flags, see PG_locked */
    struct list_head lru;       /* List of free pages */
    atomic_t _refcount;
    void *virtual;              /* Kernel virtual address (for highmem) */
} page_t;

#endif