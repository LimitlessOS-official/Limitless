/*
 * Physical Memory Manager (PMM)
 * 
 * Manages physical memory using a bitmap-based allocation scheme.
 * Tracks free and used page frames, handles allocation/deallocation.
 * 
 * Copyright (c) 2024 LimitlessOS Project
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#define PAGE_SIZE 4096
#define BITMAP_PAGES_PER_BYTE 8

// Physical memory information
static uint64_t total_memory = 0;
static uint64_t total_pages = 0;
static uint64_t used_pages = 0;
static uint64_t free_pages = 0;

// Page frame bitmap (1 bit per page)
static uint8_t *page_bitmap = NULL;
static uint64_t bitmap_size = 0;

// Page frame database entry
typedef struct {
    uint32_t ref_count;     // Reference count for COW
    uint32_t flags;         // Page flags (dirty, accessed, etc.)
    void *data;             // Private data (e.g., for page cache)
} page_frame_t;

static page_frame_t *page_frames = NULL;

// Page flags
#define PAGE_FLAG_DIRTY     0x01
#define PAGE_FLAG_ACCESSED  0x02
#define PAGE_FLAG_LOCKED    0x04
#define PAGE_FLAG_RESERVED  0x08

// Initialize physical memory manager
void pmm_init(uint64_t mem_size) {
    total_memory = mem_size;
    total_pages = mem_size / PAGE_SIZE;
    
    // Calculate bitmap size (1 bit per page)
    bitmap_size = (total_pages + 7) / 8;
    
    // Place bitmap at 1MB (assuming kernel loaded at 1MB+)
    page_bitmap = (uint8_t *)0x100000;
    
    // Initialize all pages as used
    memset(page_bitmap, 0xFF, bitmap_size);
    used_pages = total_pages;
    free_pages = 0;
    
    // Initialize page frame database after bitmap
    page_frames = (page_frame_t *)((uintptr_t)page_bitmap + bitmap_size);
    memset(page_frames, 0, total_pages * sizeof(page_frame_t));
}

// Mark a page as free
static inline void set_page_free(uint64_t page_idx) {
    uint64_t byte_idx = page_idx / 8;
    uint64_t bit_idx = page_idx % 8;
    page_bitmap[byte_idx] &= ~(1 << bit_idx);
}

// Mark a page as used
static inline void set_page_used(uint64_t page_idx) {
    uint64_t byte_idx = page_idx / 8;
    uint64_t bit_idx = page_idx % 8;
    page_bitmap[byte_idx] |= (1 << bit_idx);
}

// Check if a page is free
static inline bool is_page_free(uint64_t page_idx) {
    uint64_t byte_idx = page_idx / 8;
    uint64_t bit_idx = page_idx % 8;
    return !(page_bitmap[byte_idx] & (1 << bit_idx));
}

// Free a region of memory
void pmm_free_region(uint64_t base, uint64_t size) {
    uint64_t page_start = base / PAGE_SIZE;
    uint64_t page_count = (size + PAGE_SIZE - 1) / PAGE_SIZE;
    
    for (uint64_t i = 0; i < page_count; i++) {
        uint64_t page_idx = page_start + i;
        if (page_idx >= total_pages) break;
        
        if (!is_page_free(page_idx)) {
            set_page_free(page_idx);
            used_pages--;
            free_pages++;
        }
    }
}

// Reserve a region of memory
void pmm_reserve_region(uint64_t base, uint64_t size) {
    uint64_t page_start = base / PAGE_SIZE;
    uint64_t page_count = (size + PAGE_SIZE - 1) / PAGE_SIZE;
    
    for (uint64_t i = 0; i < page_count; i++) {
        uint64_t page_idx = page_start + i;
        if (page_idx >= total_pages) break;
        
        if (is_page_free(page_idx)) {
            set_page_used(page_idx);
            used_pages++;
            free_pages--;
        }
        page_frames[page_idx].flags |= PAGE_FLAG_RESERVED;
    }
}

// Allocate a single page frame
void *pmm_alloc_page(void) {
    // Find first free page
    for (uint64_t i = 0; i < total_pages; i++) {
        if (is_page_free(i)) {
            set_page_used(i);
            used_pages++;
            free_pages--;
            
            // Initialize page frame
            page_frames[i].ref_count = 1;
            page_frames[i].flags = 0;
            page_frames[i].data = NULL;
            
            return (void *)(i * PAGE_SIZE);
        }
    }
    
    return NULL; // Out of memory
}

// Allocate multiple contiguous pages
void *pmm_alloc_pages(uint64_t count) {
    if (count == 0) return NULL;
    
    // Find contiguous free pages
    for (uint64_t i = 0; i < total_pages; i++) {
        bool found = true;
        
        // Check if we have enough contiguous pages
        for (uint64_t j = 0; j < count; j++) {
            if (i + j >= total_pages || !is_page_free(i + j)) {
                found = false;
                break;
            }
        }
        
        if (found) {
            // Allocate all pages
            for (uint64_t j = 0; j < count; j++) {
                set_page_used(i + j);
                used_pages++;
                free_pages--;
                
                page_frames[i + j].ref_count = 1;
                page_frames[i + j].flags = 0;
                page_frames[i + j].data = NULL;
            }
            
            return (void *)(i * PAGE_SIZE);
        }
    }
    
    return NULL; // Not enough contiguous memory
}

// Free a single page frame
void pmm_free_page(void *page) {
    uint64_t page_idx = (uint64_t)page / PAGE_SIZE;
    
    if (page_idx >= total_pages) return;
    if (is_page_free(page_idx)) return; // Already free
    
    // Decrement reference count (for COW support)
    if (page_frames[page_idx].ref_count > 0) {
        page_frames[page_idx].ref_count--;
    }
    
    // Only free if no more references
    if (page_frames[page_idx].ref_count == 0) {
        set_page_free(page_idx);
        used_pages--;
        free_pages++;
        
        page_frames[page_idx].flags = 0;
        page_frames[page_idx].data = NULL;
    }
}

// Free multiple contiguous pages
void pmm_free_pages(void *page, uint64_t count) {
    for (uint64_t i = 0; i < count; i++) {
        pmm_free_page((void *)((uint64_t)page + i * PAGE_SIZE));
    }
}

// Increment page reference count (for COW)
void pmm_ref_page(void *page) {
    uint64_t page_idx = (uint64_t)page / PAGE_SIZE;
    if (page_idx >= total_pages) return;
    
    page_frames[page_idx].ref_count++;
}

// Get page reference count
uint32_t pmm_get_refcount(void *page) {
    uint64_t page_idx = (uint64_t)page / PAGE_SIZE;
    if (page_idx >= total_pages) return 0;
    
    return page_frames[page_idx].ref_count;
}

// Set page flags
void pmm_set_page_flags(void *page, uint32_t flags) {
    uint64_t page_idx = (uint64_t)page / PAGE_SIZE;
    if (page_idx >= total_pages) return;
    
    page_frames[page_idx].flags |= flags;
}

// Clear page flags
void pmm_clear_page_flags(void *page, uint32_t flags) {
    uint64_t page_idx = (uint64_t)page / PAGE_SIZE;
    if (page_idx >= total_pages) return;
    
    page_frames[page_idx].flags &= ~flags;
}

// Get page flags
uint32_t pmm_get_page_flags(void *page) {
    uint64_t page_idx = (uint64_t)page / PAGE_SIZE;
    if (page_idx >= total_pages) return 0;
    
    return page_frames[page_idx].flags;
}

// Get memory statistics
void pmm_get_stats(uint64_t *total, uint64_t *used, uint64_t *free_out) {
    if (total) *total = total_pages;
    if (used) *used = used_pages;
    if (free_out) *free_out = free_pages;
}

// Get total available memory in bytes
uint64_t pmm_get_total_memory(void) {
    return total_memory;
}

// Get free memory in bytes
uint64_t pmm_get_free_memory(void) {
    return free_pages * PAGE_SIZE;
}

// Get used memory in bytes
uint64_t pmm_get_used_memory(void) {
    return used_pages * PAGE_SIZE;
}
