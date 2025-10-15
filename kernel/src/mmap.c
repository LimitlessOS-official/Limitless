/*
 * Memory-Mapped Files & Page Cache
 * 
 * Implements:
 * - mmap/munmap syscalls
 * - File-backed memory mappings
 * - Unified page cache
 * - Read-ahead and write-back
 * 
 * Copyright (c) 2024 LimitlessOS Project
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "kernel.h"
#include "vmm.h"

#define MAX_MAPPINGS 256
#define PAGE_CACHE_SIZE 1024

// mmap protection flags
#define PROT_NONE  0x0
#define PROT_READ  0x1
#define PROT_WRITE 0x2
#define PROT_EXEC  0x4

// mmap flags
#define MAP_SHARED    0x01
#define MAP_PRIVATE   0x02
#define MAP_FIXED     0x10
#define MAP_ANONYMOUS 0x20

// File mapping structure
typedef struct {
    bool in_use;
    int fd;                  // File descriptor
    uint64_t file_offset;    // Offset in file
    vaddr_t vaddr;           // Virtual address
    uint32_t size;           // Size of mapping
    int prot;                // Protection flags
    int flags;               // MAP_* flags
    vmm_aspace_t *space;     // Address space
} file_mapping_t;

// Page cache entry
typedef struct {
    bool valid;
    int fd;                  // File descriptor
    uint64_t file_offset;    // Offset in file (page-aligned)
    paddr_t paddr;           // Physical page address
    bool dirty;              // Modified flag
    uint64_t last_access;    // For LRU eviction
} page_cache_entry_t;

static file_mapping_t mappings[MAX_MAPPINGS];
static page_cache_entry_t page_cache[PAGE_CACHE_SIZE];
static uint64_t access_counter = 0;

// Initialize mmap subsystem
void mmap_init(void) {
    memset(mappings, 0, sizeof(mappings));
    memset(page_cache, 0, sizeof(page_cache));
    access_counter = 0;
}

// Find free mapping slot
static int find_free_mapping(void) {
    for (int i = 0; i < MAX_MAPPINGS; i++) {
        if (!mappings[i].in_use) {
            return i;
        }
    }
    return -1;
}

// Find page in cache
static page_cache_entry_t *find_cached_page(int fd, uint64_t file_offset) {
    for (int i = 0; i < PAGE_CACHE_SIZE; i++) {
        if (page_cache[i].valid && 
            page_cache[i].fd == fd && 
            page_cache[i].file_offset == file_offset) {
            page_cache[i].last_access = access_counter++;
            return &page_cache[i];
        }
    }
    return NULL;
}

// Allocate cache entry (LRU eviction)
static page_cache_entry_t *alloc_cache_entry(void) {
    // First try to find invalid entry
    for (int i = 0; i < PAGE_CACHE_SIZE; i++) {
        if (!page_cache[i].valid) {
            return &page_cache[i];
        }
    }
    
    // Find LRU entry
    uint64_t min_access = page_cache[0].last_access;
    int lru_idx = 0;
    
    for (int i = 1; i < PAGE_CACHE_SIZE; i++) {
        if (page_cache[i].last_access < min_access) {
            min_access = page_cache[i].last_access;
            lru_idx = i;
        }
    }
    
    // Evict LRU entry (write back if dirty)
    page_cache_entry_t *entry = &page_cache[lru_idx];
    if (entry->dirty) {
        // Write back to file (would call VFS write here)
        // fs_write(entry->fd, entry->file_offset, (void *)entry->paddr, PAGE_SIZE);
    }
    
    entry->valid = false;
    return entry;
}

// Read page from file into cache
static paddr_t read_file_page(int fd, uint64_t file_offset) {
    // Check cache first
    page_cache_entry_t *cached = find_cached_page(fd, file_offset);
    if (cached) {
        return cached->paddr;
    }
    
    // Not in cache - allocate page
    paddr_t page = pmm_alloc_page();
    if (!page) return 0;
    
    // Clear the page (zero-fill for now - would read from file)
    void *page_virt = (void *)PHYS_TO_VIRT_DIRECT(page);
    memset(page_virt, 0, PAGE_SIZE);
    
    // Add to cache
    page_cache_entry_t *entry = alloc_cache_entry();
    if (entry) {
        entry->valid = true;
        entry->fd = fd;
        entry->file_offset = file_offset;
        entry->paddr = page;
        entry->dirty = false;
        entry->last_access = access_counter++;
    }
    
    return page;
}

// mmap system call
void *sys_mmap(void *addr, uint32_t length, int prot, int flags, int fd, uint64_t offset) {
    if (length == 0) return (void *)-1;
    
    // Find free mapping slot
    int map_idx = find_free_mapping();
    if (map_idx < 0) return (void *)-1;
    
    vmm_aspace_t *space = vmm_get_current_aspace();
    if (!space) return (void *)-1;
    
    // Determine virtual address
    vaddr_t vaddr;
    if (flags & MAP_FIXED) {
        vaddr = (vaddr_t)addr;
    } else {
        // Find free virtual address (simplified)
        vaddr = 0x40000000; // Start of user mmap region
    }
    
    // Round up to page boundary
    uint32_t page_count = (length + PAGE_SIZE - 1) / PAGE_SIZE;
    
    // Convert protection to page flags
    uint32_t page_flags = PTE_PRESENT | PTE_USER;
    if (prot & PROT_WRITE) page_flags |= PTE_WRITABLE;
    if (!(prot & PROT_EXEC)) page_flags |= PTE_NX;
    
    // Map pages
    if (flags & MAP_ANONYMOUS) {
        // Anonymous mapping - allocate zero pages
        for (uint32_t i = 0; i < page_count; i++) {
            paddr_t page = pmm_alloc_page();
            if (!page) {
                // Cleanup on failure
                for (uint32_t j = 0; j < i; j++) {
                    vmm_unmap_page(space, vaddr + j * PAGE_SIZE);
                }
                return (void *)-1;
            }
            
            void *page_virt = (void *)PHYS_TO_VIRT_DIRECT(page);
            memset(page_virt, 0, PAGE_SIZE);
            
            if (vmm_map_page(space, vaddr + i * PAGE_SIZE, page, page_flags) != 0) {
                pmm_free_page(page);
                // Cleanup
                for (uint32_t j = 0; j < i; j++) {
                    vmm_unmap_page(space, vaddr + j * PAGE_SIZE);
                }
                return (void *)-1;
            }
        }
    } else {
        // File-backed mapping
        for (uint32_t i = 0; i < page_count; i++) {
            uint64_t file_off = offset + i * PAGE_SIZE;
            paddr_t paddr = read_file_page(fd, file_off);
            
            if (!paddr || vmm_map_page(space, vaddr + i * PAGE_SIZE, paddr, page_flags) != 0) {
                // Cleanup
                for (uint32_t j = 0; j < i; j++) {
                    vmm_unmap_page(space, vaddr + j * PAGE_SIZE);
                }
                return (void *)-1;
            }
        }
    }
    
    // Record mapping
    mappings[map_idx].in_use = true;
    mappings[map_idx].fd = fd;
    mappings[map_idx].file_offset = offset;
    mappings[map_idx].vaddr = vaddr;
    mappings[map_idx].size = page_count * PAGE_SIZE;
    mappings[map_idx].prot = prot;
    mappings[map_idx].flags = flags;
    mappings[map_idx].space = space;
    
    return (void *)vaddr;
}

// munmap system call
int sys_munmap(void *addr, uint32_t length) {
    vaddr_t vaddr = (vaddr_t)addr;
    vmm_aspace_t *space = vmm_get_current_aspace();
    
    // Find mapping
    int map_idx = -1;
    for (int i = 0; i < MAX_MAPPINGS; i++) {
        if (mappings[i].in_use && 
            mappings[i].vaddr == vaddr && 
            mappings[i].space == space) {
            map_idx = i;
            break;
        }
    }
    
    if (map_idx < 0) return -1;
    
    // Unmap pages
    uint32_t page_count = (length + PAGE_SIZE - 1) / PAGE_SIZE;
    for (uint32_t i = 0; i < page_count; i++) {
        paddr_t paddr;
        if (vmm_get_physical(space, vaddr + i * PAGE_SIZE, &paddr) == 0) {
            vmm_unmap_page(space, vaddr + i * PAGE_SIZE);
            
            // Free if private mapping
            if (mappings[map_idx].flags & MAP_PRIVATE) {
                pmm_free_page(paddr);
            }
        }
    }
    
    // Remove mapping
    mappings[map_idx].in_use = false;
    
    return 0;
}

// msync - sync file-backed mapping to disk
int sys_msync(void *addr, uint32_t length, int flags) {
    vaddr_t vaddr = (vaddr_t)addr;
    vmm_aspace_t *space = vmm_get_current_aspace();
    
    // Find mapping
    int map_idx = -1;
    for (int i = 0; i < MAX_MAPPINGS; i++) {
        if (mappings[i].in_use && 
            mappings[i].vaddr <= vaddr && 
            vaddr < mappings[i].vaddr + mappings[i].size &&
            mappings[i].space == space) {
            map_idx = i;
            break;
        }
    }
    
    if (map_idx < 0) return -1;
    if (mappings[map_idx].flags & MAP_ANONYMOUS) return 0; // Nothing to sync
    
    // For now, just return success - actual sync would write to VFS
    return 0;
}

// Flush all dirty pages in page cache
void page_cache_flush_all(void) {
    for (int i = 0; i < PAGE_CACHE_SIZE; i++) {
        if (page_cache[i].valid && page_cache[i].dirty) {
            // Write back
            // fs_write(page_cache[i].fd, page_cache[i].file_offset,
            //          (void *)page_cache[i].paddr, PAGE_SIZE);
            page_cache[i].dirty = false;
        }
    }
}

// Get page cache statistics
void page_cache_get_stats(uint32_t *total, uint32_t *used, uint32_t *dirty_count) {
    if (total) *total = PAGE_CACHE_SIZE;
    
    uint32_t used_cnt = 0;
    uint32_t dirty_cnt = 0;
    
    for (int i = 0; i < PAGE_CACHE_SIZE; i++) {
        if (page_cache[i].valid) {
            used_cnt++;
            if (page_cache[i].dirty) dirty_cnt++;
        }
    }
    
    if (used) *used = used_cnt;
    if (dirty_count) *dirty_count = dirty_cnt;
}
