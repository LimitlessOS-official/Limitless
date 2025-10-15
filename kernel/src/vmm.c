/*
 * Virtual Memory Manager (VMM)
 * 
 * Manages virtual memory using paging (4KB pages).
 * Handles page tables, address spaces, and TLB management.
 * 
 * Copyright (c) 2024 LimitlessOS Project
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "pmm.h"

#define PAGE_SIZE 4096
#define PAGE_ENTRIES 1024

// Page directory/table entry flags
#define PAGE_PRESENT    0x001
#define PAGE_WRITE      0x002
#define PAGE_USER       0x004
#define PAGE_WRITETHROUGH 0x008
#define PAGE_CACHEDISABLE 0x010
#define PAGE_ACCESSED   0x020
#define PAGE_DIRTY      0x040
#define PAGE_SIZE_4MB   0x080
#define PAGE_GLOBAL     0x100
#define PAGE_COW        0x200  // Custom flag for copy-on-write

// Page directory and table types
typedef uint32_t page_table_entry_t;
typedef uint32_t page_dir_entry_t;

typedef struct {
    page_table_entry_t entries[PAGE_ENTRIES];
} page_table_t;

typedef struct {
    page_dir_entry_t entries[PAGE_ENTRIES];
} page_directory_t;

// Address space structure
typedef struct address_space {
    page_directory_t *page_dir;
    uint32_t page_dir_phys;
    struct address_space *next;
} address_space_t;

// Current address space
static address_space_t *current_space = NULL;
static address_space_t *kernel_space = NULL;

// Get page directory index from virtual address
static inline uint32_t pd_index(uint32_t vaddr) {
    return (vaddr >> 22) & 0x3FF;
}

// Get page table index from virtual address
static inline uint32_t pt_index(uint32_t vaddr) {
    return (vaddr >> 12) & 0x3FF;
}

// Get page-aligned address
static inline uint32_t page_align(uint32_t addr) {
    return addr & ~(PAGE_SIZE - 1);
}

// Invalidate TLB entry
static inline void tlb_invalidate(uint32_t vaddr) {
    __asm__ volatile("invlpg (%0)" : : "r"(vaddr) : "memory");
}

// Reload CR3 (flush entire TLB)
static inline void tlb_flush_all(void) {
    uint32_t cr3;
    __asm__ volatile("mov %%cr3, %0" : "=r"(cr3));
    __asm__ volatile("mov %0, %%cr3" : : "r"(cr3));
}

// Load page directory
static inline void load_page_directory(uint32_t page_dir_phys) {
    __asm__ volatile("mov %0, %%cr3" : : "r"(page_dir_phys));
}

// Enable paging
static inline void enable_paging(void) {
    uint32_t cr0;
    __asm__ volatile("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000; // Set PG bit
    __asm__ volatile("mov %0, %%cr0" : : "r"(cr0));
}

// Initialize VMM
void vmm_init(void) {
    // Create kernel address space
    kernel_space = (address_space_t *)pmm_alloc_page();
    kernel_space->page_dir = (page_directory_t *)pmm_alloc_page();
    kernel_space->page_dir_phys = (uint32_t)kernel_space->page_dir;
    kernel_space->next = NULL;
    
    // Clear page directory
    memset(kernel_space->page_dir, 0, PAGE_SIZE);
    
    // Identity map first 4MB for kernel
    page_table_t *kernel_pt = (page_table_t *)pmm_alloc_page();
    memset(kernel_pt, 0, PAGE_SIZE);
    
    for (uint32_t i = 0; i < PAGE_ENTRIES; i++) {
        kernel_pt->entries[i] = (i * PAGE_SIZE) | PAGE_PRESENT | PAGE_WRITE;
    }
    
    kernel_space->page_dir->entries[0] = (uint32_t)kernel_pt | PAGE_PRESENT | PAGE_WRITE;
    
    // Load kernel page directory
    load_page_directory(kernel_space->page_dir_phys);
    enable_paging();
    
    current_space = kernel_space;
}

// Create new address space
address_space_t *vmm_create_address_space(void) {
    address_space_t *space = (address_space_t *)pmm_alloc_page();
    if (!space) return NULL;
    
    space->page_dir = (page_directory_t *)pmm_alloc_page();
    if (!space->page_dir) {
        pmm_free_page(space);
        return NULL;
    }
    
    space->page_dir_phys = (uint32_t)space->page_dir;
    space->next = NULL;
    
    // Clear page directory
    memset(space->page_dir, 0, PAGE_SIZE);
    
    // Copy kernel mappings (first entry)
    space->page_dir->entries[0] = kernel_space->page_dir->entries[0];
    
    return space;
}

// Destroy address space
void vmm_destroy_address_space(address_space_t *space) {
    if (!space || space == kernel_space) return;
    
    // Free all user page tables (skip kernel mapping)
    for (uint32_t i = 1; i < PAGE_ENTRIES; i++) {
        if (space->page_dir->entries[i] & PAGE_PRESENT) {
            page_table_t *pt = (page_table_t *)(space->page_dir->entries[i] & ~0xFFF);
            
            // Free all mapped pages
            for (uint32_t j = 0; j < PAGE_ENTRIES; j++) {
                if (pt->entries[j] & PAGE_PRESENT) {
                    void *page = (void *)(pt->entries[j] & ~0xFFF);
                    pmm_free_page(page);
                }
            }
            
            pmm_free_page(pt);
        }
    }
    
    pmm_free_page(space->page_dir);
    pmm_free_page(space);
}

// Switch to address space
void vmm_switch_address_space(address_space_t *space) {
    if (!space) return;
    
    current_space = space;
    load_page_directory(space->page_dir_phys);
}

// Get current address space
address_space_t *vmm_get_current_space(void) {
    return current_space;
}

// Map a virtual page to a physical page
bool vmm_map_page(address_space_t *space, uint32_t vaddr, uint32_t paddr, uint32_t flags) {
    if (!space) space = current_space;
    
    uint32_t pd_idx = pd_index(vaddr);
    uint32_t pt_idx = pt_index(vaddr);
    
    // Get or create page table
    page_table_t *pt;
    if (!(space->page_dir->entries[pd_idx] & PAGE_PRESENT)) {
        pt = (page_table_t *)pmm_alloc_page();
        if (!pt) return false;
        
        memset(pt, 0, PAGE_SIZE);
        space->page_dir->entries[pd_idx] = (uint32_t)pt | PAGE_PRESENT | PAGE_WRITE | PAGE_USER;
    } else {
        pt = (page_table_t *)(space->page_dir->entries[pd_idx] & ~0xFFF);
    }
    
    // Map the page
    pt->entries[pt_idx] = page_align(paddr) | flags | PAGE_PRESENT;
    
    // Invalidate TLB
    if (space == current_space) {
        tlb_invalidate(vaddr);
    }
    
    return true;
}

// Unmap a virtual page
void vmm_unmap_page(address_space_t *space, uint32_t vaddr) {
    if (!space) space = current_space;
    
    uint32_t pd_idx = pd_index(vaddr);
    uint32_t pt_idx = pt_index(vaddr);
    
    if (!(space->page_dir->entries[pd_idx] & PAGE_PRESENT)) return;
    
    page_table_t *pt = (page_table_t *)(space->page_dir->entries[pd_idx] & ~0xFFF);
    
    if (pt->entries[pt_idx] & PAGE_PRESENT) {
        pt->entries[pt_idx] = 0;
        
        if (space == current_space) {
            tlb_invalidate(vaddr);
        }
    }
}

// Get physical address from virtual address
uint32_t vmm_get_physical_address(address_space_t *space, uint32_t vaddr) {
    if (!space) space = current_space;
    
    uint32_t pd_idx = pd_index(vaddr);
    uint32_t pt_idx = pt_index(vaddr);
    
    if (!(space->page_dir->entries[pd_idx] & PAGE_PRESENT)) {
        return 0;
    }
    
    page_table_t *pt = (page_table_t *)(space->page_dir->entries[pd_idx] & ~0xFFF);
    
    if (!(pt->entries[pt_idx] & PAGE_PRESENT)) {
        return 0;
    }
    
    return (pt->entries[pt_idx] & ~0xFFF) | (vaddr & 0xFFF);
}

// Check if page is mapped
bool vmm_is_mapped(address_space_t *space, uint32_t vaddr) {
    return vmm_get_physical_address(space, vaddr) != 0;
}

// Set page flags
void vmm_set_page_flags(address_space_t *space, uint32_t vaddr, uint32_t flags) {
    if (!space) space = current_space;
    
    uint32_t pd_idx = pd_index(vaddr);
    uint32_t pt_idx = pt_index(vaddr);
    
    if (!(space->page_dir->entries[pd_idx] & PAGE_PRESENT)) return;
    
    page_table_t *pt = (page_table_t *)(space->page_dir->entries[pd_idx] & ~0xFFF);
    
    if (pt->entries[pt_idx] & PAGE_PRESENT) {
        pt->entries[pt_idx] |= flags;
        
        if (space == current_space) {
            tlb_invalidate(vaddr);
        }
    }
}

// Clear page flags
void vmm_clear_page_flags(address_space_t *space, uint32_t vaddr, uint32_t flags) {
    if (!space) space = current_space;
    
    uint32_t pd_idx = pd_index(vaddr);
    uint32_t pt_idx = pt_index(vaddr);
    
    if (!(space->page_dir->entries[pd_idx] & PAGE_PRESENT)) return;
    
    page_table_t *pt = (page_table_t *)(space->page_dir->entries[pd_idx] & ~0xFFF);
    
    if (pt->entries[pt_idx] & PAGE_PRESENT) {
        pt->entries[pt_idx] &= ~flags;
        
        if (space == current_space) {
            tlb_invalidate(vaddr);
        }
    }
}

// Get page flags
uint32_t vmm_get_page_flags(address_space_t *space, uint32_t vaddr) {
    if (!space) space = current_space;
    
    uint32_t pd_idx = pd_index(vaddr);
    uint32_t pt_idx = pt_index(vaddr);
    
    if (!(space->page_dir->entries[pd_idx] & PAGE_PRESENT)) return 0;
    
    page_table_t *pt = (page_table_t *)(space->page_dir->entries[pd_idx] & ~0xFFF);
    
    if (!(pt->entries[pt_idx] & PAGE_PRESENT)) return 0;
    
    return pt->entries[pt_idx] & 0xFFF;
}

// Allocate and map virtual memory region
void *vmm_alloc_region(address_space_t *space, uint32_t vaddr, uint32_t size, uint32_t flags) {
    if (!space) space = current_space;
    
    uint32_t page_count = (size + PAGE_SIZE - 1) / PAGE_SIZE;
    uint32_t current_vaddr = page_align(vaddr);
    
    for (uint32_t i = 0; i < page_count; i++) {
        void *page = pmm_alloc_page();
        if (!page) {
            // Cleanup on failure
            for (uint32_t j = 0; j < i; j++) {
                vmm_unmap_page(space, vaddr + j * PAGE_SIZE);
            }
            return NULL;
        }
        
        if (!vmm_map_page(space, current_vaddr, (uint32_t)page, flags)) {
            pmm_free_page(page);
            // Cleanup
            for (uint32_t j = 0; j < i; j++) {
                vmm_unmap_page(space, vaddr + j * PAGE_SIZE);
            }
            return NULL;
        }
        
        current_vaddr += PAGE_SIZE;
    }
    
    return (void *)vaddr;
}

// Free and unmap virtual memory region
void vmm_free_region(address_space_t *space, uint32_t vaddr, uint32_t size) {
    if (!space) space = current_space;
    
    uint32_t page_count = (size + PAGE_SIZE - 1) / PAGE_SIZE;
    
    for (uint32_t i = 0; i < page_count; i++) {
        uint32_t current_vaddr = vaddr + i * PAGE_SIZE;
        uint32_t paddr = vmm_get_physical_address(space, current_vaddr);
        
        if (paddr) {
            pmm_free_page((void *)paddr);
            vmm_unmap_page(space, current_vaddr);
        }
    }
}
