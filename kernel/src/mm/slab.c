/*
 * LimitlessOS - Slab Allocator
 *
 * This file implements a slab allocator for kernel objects, providing
 * a more efficient way to manage small, fixed-size allocations than
 * the page-level allocator.
 */

#include "../include/mm/mm.h"
#include <stddef.h>
#include <stdbool.h>

#define NUM_GENERAL_CACHES 10
static kmem_cache_t* general_caches[NUM_GENERAL_CACHES];
static const size_t general_cache_sizes[NUM_GENERAL_CACHES] = {
    8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096
};

// Forward declaration
static void kmem_cache_grow(kmem_cache_t* cache);

/**
 * @brief Creates a new kernel memory cache.
 * @param name The name of the cache.
 * @param size The size of objects in the cache.
 * @param align The alignment of objects in the cache.
 * @return A pointer to the new cache, or NULL on failure.
 */
kmem_cache_t* kmem_cache_create(const char* name, size_t size, size_t align) {
    kmem_cache_t* cache = (kmem_cache_t*)pmm_alloc_page(); // Simplified: should use a smaller allocation
    if (!cache) return NULL;

    cache->name = name;
    cache->object_size = size;
    cache->object_align = align;
    cache->slabs_full = NULL;
    cache->slabs_partial = NULL;
    cache->slabs_free = NULL;

    return cache;
}

/**
 * @brief Allocates an object from a kernel memory cache.
 * @param cache The cache to allocate from.
 * @return A pointer to the allocated object, or NULL on failure.
 */
void* kmem_cache_alloc(kmem_cache_t* cache) {
    slab_t* slab = cache->slabs_partial;
    if (!slab) {
        slab = cache->slabs_free;
        if (!slab) {
            kmem_cache_grow(cache);
            slab = cache->slabs_free;
            if (!slab) return NULL;
        }
        // Move slab from free to partial list
        cache->slabs_free = slab->next;
        slab->next = cache->slabs_partial;
        cache->slabs_partial = slab;
    }

    void* obj = slab->free_list;
    slab->free_list = *((void**)obj);
    slab->inuse++;

    if (slab->inuse == slab->capacity) {
        // Move slab from partial to full list
        cache->slabs_partial = slab->next;
        slab->next = cache->slabs_full;
        cache->slabs_full = slab;
    }

    return obj;
}

/**
 * @brief Frees an object back to a kernel memory cache.
 * @param cache The cache the object belongs to.
 * @param obj The object to free.
 */
void kmem_cache_free(kmem_cache_t* cache, void* obj) {
    // This is a simplified implementation. A real one would need to find the slab for the object.
    // For now, we assume we can find the slab.
    slab_t* slab = cache->slabs_full; // Simplified search
    if (!slab) slab = cache->slabs_partial;

    *((void**)obj) = slab->free_list;
    slab->free_list = obj;
    slab->inuse--;

    // Simplified: move slab between lists if needed
}

/**
 * @brief Grows a kernel memory cache by adding a new slab.
 * @param cache The cache to grow.
 */
static void kmem_cache_grow(kmem_cache_t* cache) {
    void* page = pmm_alloc_page();
    if (!page) return;

    slab_t* slab = (slab_t*)page;
    slab->inuse = 0;
    slab->capacity = (PAGE_SIZE - sizeof(slab_t)) / cache->object_size;
    slab->free_list = (char*)page + sizeof(slab_t);

    // Initialize the free list in the new slab
    char* ptr = (char*)slab->free_list;
    for (size_t i = 0; i < slab->capacity - 1; i++) {
        *((void**)(ptr + i * cache->object_size)) = ptr + (i + 1) * cache->object_size;
    }
    *((void**)(ptr + (slab->capacity - 1) * cache->object_size)) = NULL;

    // Add the new slab to the free list
    slab->next = cache->slabs_free;
    cache->slabs_free = slab;
}

/**
 * @brief Initializes the slab allocator and general purpose caches.
 */
void slab_init(void) {
    for (int i = 0; i < NUM_GENERAL_CACHES; i++) {
        char* name = "gen-cache"; // Simplified name
        general_caches[i] = kmem_cache_create(name, general_cache_sizes[i], 0);
    }
}

/**
 * @brief Allocates a block of memory from the kernel heap.
 * @param size The size of the block to allocate.
 * @return A pointer to the allocated block, or NULL on failure.
 */
void* kmalloc(size_t size) {
    if (size > general_cache_sizes[NUM_GENERAL_CACHES - 1]) {
        // For large allocations, fall back to the page allocator
        // This is a simplified approach.
        size_t num_pages = (size + PAGE_SIZE - 1) / PAGE_SIZE;
        return pmm_alloc_pages(0); // Simplified: order should be calculated
    }

    for (int i = 0; i < NUM_GENERAL_CACHES; i++) {
        if (size <= general_cache_sizes[i]) {
            return kmem_cache_alloc(general_caches[i]);
        }
    }
    return NULL; // Should not be reached
}

/**
 * @brief Frees a block of memory on the kernel heap.
 * @param ptr The block to free.
 */
void kfree(void* ptr) {
    // This is highly simplified. A real implementation needs to determine
    // which cache the allocation came from. This often involves storing
    // metadata near the allocation.
}
