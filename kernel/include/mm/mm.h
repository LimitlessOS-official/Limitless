/*
 * LimitlessOS - Memory Management Header
 *
 * This is the main header file for the memory management subsystem.
 * It defines the APIs for the Physical Memory Manager (PMM),
 * Virtual Memory Manager (VMM), and the Slab Allocator.
 */

#ifndef KERNEL_MM_H
#define KERNEL_MM_H


#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "vmm.h"

#define PAGE_SIZE 4096

// --- Physical Memory Manager (PMM) ---

void pmm_init();
// Use canonical types from vmm.h
paddr_t pmm_alloc_page(void);
void pmm_free_page(paddr_t paddr);
paddr_t pmm_alloc_pages(size_t pages);
void pmm_free_pages(paddr_t paddr, size_t pages);


// --- Virtual Memory Manager (VMM) ---

// Page table entry flags
#define VMM_FLAG_PRESENT  (1ULL << 0)
#define VMM_FLAG_WRITE    (1ULL << 1)
#define VMM_FLAG_USER     (1ULL << 2)

// All VMM types and prototypes are now in vmm.h

// Kernel heap allocators (from vmm.h)
void* vmm_kmalloc(size_t size, size_t align);
void  vmm_kfree(void* ptr, size_t size);
uint64_t* vmm_get_kernel_pml4();


// --- Slab Allocator ---

typedef struct slab_s {
    struct slab_s* next;
    void* free_list;
    size_t inuse;
    size_t capacity;
} slab_t;

typedef struct {
    const char* name;
    size_t object_size;
    size_t object_align;
    slab_t* slabs_full;
    slab_t* slabs_partial;
    slab_t* slabs_free;
} kmem_cache_t;

void slab_init(void);
kmem_cache_t* kmem_cache_create(const char* name, size_t size, size_t align);
void* kmem_cache_alloc(kmem_cache_t* cache);
void kmem_cache_free(kmem_cache_t* cache, void* obj);

// General purpose kernel allocation functions
void* kmalloc(size_t size);
void kfree(void* ptr);

#endif // KERNEL_MM_H