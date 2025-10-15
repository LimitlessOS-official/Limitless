/*
 * LimitlessOS - Physical Memory Manager (Buddy Allocator)
 *
 * This file implements a binary buddy allocator for managing physical page frames.
 * It divides memory into power-of-2-sized blocks and maintains free lists for
 * each block size (order).
 */

#include <linux/list.h>
#include <linux/spinlock.h>
#include <linux/atomic.h>
#include <linux/mm_types.h>
#include <linux/mmzone.h>
#include "mm/mm.h"

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

// This would be defined in a boot-time memory map parser
extern uint64_t memory_map_size;
extern uint64_t memory_map_addr;
extern void* kernel_end;

#define PAGE_MASK (~(PAGE_SIZE - 1))

static inline void set_page_address(page_t *page, void *addr) {
    page->virtual = addr;
}

static inline void *get_page_address(page_t *page) {
    return page->virtual;
}

typedef struct {
    spinlock_t lock;
    pglist_data_t node; // For now, we assume a single NUMA node
    page_t* page_map;
    uint64_t total_pages;
    uint64_t free_pages;
} pmm_state_t;

static pmm_state_t pmm_state;

void pmm_init() {
    // Simplified: We just take all memory after the kernel for the PMM
    // A real implementation would parse the memory map from the bootloader
    uint64_t total_mem = 1024 * 1024 * 1024; // Assume 1GB for now
    pmm_state.total_pages = total_mem / PAGE_SIZE;
    pmm_state.free_pages = pmm_state.total_pages;

    uint64_t kernel_end_aligned = ((uint64_t)kernel_end + PAGE_SIZE - 1) & PAGE_MASK;
    
    uint64_t page_map_size = pmm_state.total_pages * sizeof(page_t);
    pmm_state.page_map = (page_t*)kernel_end_aligned;
    uint64_t pmm_area_end = kernel_end_aligned + page_map_size;
    memset(pmm_state.page_map, 0, page_map_size);

    // Initialize the buddy allocator structures
    pg_data_t* node = &pmm_state.node;
    node->node_id = 0;
    node->node_start_pfn = 0;
    node->node_spanned_pages = pmm_state.total_pages;
    node->nr_zones = 1; // Just ZONE_NORMAL for now
    
    zone_t* zone = &node->node_zones[ZONE_NORMAL];
    zone->name = "Normal";
    zone->zone_start_pfn = 0;
    zone->spanned_pages = pmm_state.total_pages;
    spin_lock_init(&zone->lock);

    for (int i = 0; i < MAX_ORDER; i++) {
        INIT_LIST_HEAD(&zone->free_area[i].free_list);
        zone->free_area[i].nr_free = 0;
    }

    // Add free pages to the buddy allocator
    // We mark pages used by the kernel and PMM as reserved
    uint64_t reserved_pages = pmm_area_end / PAGE_SIZE;
    pmm_state.free_pages = 0;
    for (uint64_t pfn = reserved_pages; pfn < pmm_state.total_pages; pfn++) {
        page_t* page = &pmm_state.page_map[pfn];
        atomic_set(&page->_refcount, 0);
        set_page_address(page, (void*)(pfn * PAGE_SIZE));
        INIT_LIST_HEAD(&page->lru);
        list_add(&page->lru, &zone->free_area[0].free_list);
        zone->free_area[0].nr_free++;
        pmm_state.free_pages++;
    }

    node->node_present_pages = pmm_state.free_pages;
    zone->present_pages = pmm_state.free_pages;
    zone->managed_pages = pmm_state.free_pages;
}


/**
 * @brief Allocates a block of contiguous physical pages.
 * 
 * @param order The order of the allocation (2^order pages).
 * @return A pointer to the first page descriptor, or NULL if allocation fails.
 */
page_t* alloc_pages(unsigned int order) {
    if (order >= MAX_ORDER) {
        return NULL;
    }

    spin_lock(&pmm_state.node.node_zones[ZONE_NORMAL].lock);

    struct list_head* list = &pmm_state.node.node_zones[ZONE_NORMAL].free_area[0].free_list;
    if (list_empty(list)) {
        spin_unlock(&pmm_state.node.node_zones[ZONE_NORMAL].lock);
        return NULL; // Out of memory
    }

    page_t* page = list_entry(list->next, page_t, lru);
    list_del(&page->lru);

    // Mark page as used
    atomic_set(&page->_refcount, 1);
    page->flags |= (1 << PG_locked);

    pmm_state.node.node_zones[ZONE_NORMAL].free_area[0].nr_free--;
    pmm_state.free_pages--;

    spin_unlock(&pmm_state.node.node_zones[ZONE_NORMAL].lock);

    return page;
}

/**
 * @brief Frees a block of contiguous physical pages.
 * 
 * @param page Pointer to the first page descriptor.
 * @param order The order of the block (2^order pages).
 */
void free_pages(page_t* page, unsigned int order) {
    if (!page) return;

    spin_lock(&pmm_state.node.node_zones[ZONE_NORMAL].lock);

    // Clear flags and add back to free list
    page->flags &= ~(1 << PG_locked);
    atomic_set(&page->_refcount, 0);
    
    list_add(&page->lru, &pmm_state.node.node_zones[ZONE_NORMAL].free_area[order].free_list);
    pmm_state.node.node_zones[ZONE_NORMAL].free_area[order].nr_free++;
    pmm_state.free_pages += (1 << order);

    spin_unlock(&pmm_state.node.node_zones[ZONE_NORMAL].lock);
}

/**
 * @brief Allocates a single physical page.
 * 
 * @return A virtual address pointer to the allocated page, or NULL on failure.
 */
// Returns physical address of allocated page
paddr_t pmm_alloc_page(void) {
    page_t* page = alloc_pages(0);
    if (!page) {
        return 0;
    }
    return (paddr_t)((uint64_t)get_page_address(page));
}

/**
 * @brief Frees a single physical page.
 * 
 * @param page_addr The virtual address of the page to free.
 */
void pmm_free_page(paddr_t paddr) {
    if (!paddr) return;
    uint64_t pfn = paddr / PAGE_SIZE;
    free_pages(&pmm_state.page_map[pfn], 0);
}

/**
 * @brief Allocates multiple contiguous physical pages.
 * 
 * @param order The order of the allocation (2^order pages).
 * @return A virtual address pointer to the start of the block, or NULL on failure.
 */
paddr_t pmm_alloc_pages(size_t pages) {
    // Find the smallest order >= pages
    unsigned int order = 0;
    while ((1U << order) < pages) order++;
    page_t* page = alloc_pages(order);
    if (!page) {
        return 0;
    }
    return (paddr_t)((uint64_t)get_page_address(page));
}

/**
 * @brief Frees multiple contiguous physical pages.
 * 
 * @param start The starting virtual address of the block.
 * @param order The order of the block.
 */
void pmm_free_pages(paddr_t paddr, size_t pages) {
    if (!paddr) return;
    // Find the smallest order >= pages
    unsigned int order = 0;
    while ((1U << order) < pages) order++;
    uint64_t pfn = paddr / PAGE_SIZE;
    free_pages(&pmm_state.page_map[pfn], order);
}
