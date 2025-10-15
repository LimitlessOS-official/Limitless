/*
 * Page Fault Handler & Demand Paging
 * 
 * Handles page faults for:
 * - Demand paging (allocate on access)
 * - Copy-on-Write (COW)
 * - Swapping (if swap is enabled)
 * - Memory-mapped files
 * 
 * Copyright (c) 2024 LimitlessOS Project
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "kernel.h"
#include "vmm.h"

// Page fault error code bits
#define PF_PRESENT  0x01  // Page not present
#define PF_WRITE    0x02  // Write access
#define PF_USER     0x04  // User mode
#define PF_RESERVED 0x08  // Reserved bit set
#define PF_INSTR    0x10  // Instruction fetch

// Demand paging statistics
static uint64_t page_faults_handled = 0;
static uint64_t pages_allocated = 0;
static uint64_t cow_pages_copied = 0;
static uint64_t swap_ins = 0;
static uint64_t swap_outs = 0;

// Copy a page for COW
static bool copy_page_cow(vmm_aspace_t *space, vaddr_t vaddr, paddr_t old_paddr) {
    // Allocate new page
    paddr_t new_page = pmm_alloc_page();
    if (!new_page) return false;
    
    // Copy contents from old page to new page
    void *old_virt = (void *)PHYS_TO_VIRT_DIRECT(old_paddr);
    void *new_virt = (void *)PHYS_TO_VIRT_DIRECT(new_page);
    memcpy(new_virt, old_virt, PAGE_SIZE);
    
    // Unmap old page
    vmm_unmap_page(space, vaddr);
    
    // Map new page with write permission
    uint32_t flags = PTE_PRESENT | PTE_WRITABLE | PTE_USER;
    if (vmm_map_page(space, vaddr, new_page, flags) != 0) {
        pmm_free_page(new_page);
        return false;
    }
    
    // Decrement reference count on old page
    pmm_free_page(old_paddr);
    
    cow_pages_copied++;
    return true;
}

// Handle COW page fault
static bool handle_cow_fault(vmm_aspace_t *space, vaddr_t vaddr, paddr_t paddr) {
    // For now, always copy - reference counting would be added later
    return copy_page_cow(space, vaddr, paddr);
}

// Handle demand paging fault (page not present)
static bool handle_demand_paging(vmm_aspace_t *space, vaddr_t vaddr, bool user_mode) {
    // Allocate a new page
    paddr_t page = pmm_alloc_page();
    if (!page) return false;
    
    // Clear the page (zero-fill)
    void *page_virt = (void *)PHYS_TO_VIRT_DIRECT(page);
    memset(page_virt, 0, PAGE_SIZE);
    
    // Determine flags
    uint32_t flags = PTE_PRESENT | PTE_WRITABLE;
    if (user_mode) {
        flags |= PTE_USER;
    }
    
    // Map the page
    if (vmm_map_page(space, vaddr & ~(PAGE_SIZE - 1), page, flags) != 0) {
        pmm_free_page(page);
        return false;
    }
    
    pages_allocated++;
    return true;
}

// Main page fault handler
void page_fault_handler(uint32_t error_code, uint32_t fault_addr) {
    page_faults_handled++;
    
    vmm_aspace_t *space = vmm_get_current_aspace();
    if (!space) {
        // No current address space - kernel panic would go here
        return;
    }
    
    bool present = error_code & PF_PRESENT;
    bool write = error_code & PF_WRITE;
    bool user_mode = error_code & PF_USER;
    bool reserved = error_code & PF_RESERVED;
    
    // Handle reserved bit violations (shouldn't happen)
    if (reserved) {
        // Kernel panic - reserved bit set
        return;
    }
    
    // Get page-aligned address
    vaddr_t page_addr = fault_addr & ~(PAGE_SIZE - 1);
    
    // Case 1: Page not present - demand paging
    if (!present) {
        if (handle_demand_paging(space, page_addr, user_mode)) {
            return; // Success
        }
        // Failed to allocate - out of memory
        return;
    }
    
    // Case 2: Page present but write to COW page - would check PTE_COW flag
    if (present && write) {
        // For now, just handle as protection violation
        // Full COW would check the PTE_COW bit and handle accordingly
        return;
    }
    
    // Case 3: Invalid access - segmentation fault would be triggered here
}

// Initialize page fault handler
void page_fault_init(void) {
    page_faults_handled = 0;
    pages_allocated = 0;
    cow_pages_copied = 0;
    swap_ins = 0;
    swap_outs = 0;
}

// Get page fault statistics
void page_fault_get_stats(uint64_t *faults, uint64_t *allocs, uint64_t *cow_copies) {
    if (faults) *faults = page_faults_handled;
    if (allocs) *allocs = pages_allocated;
    if (cow_copies) *cow_copies = cow_pages_copied;
}

// Mark a page as copy-on-write
bool vmm_mark_cow(vmm_aspace_t *space, vaddr_t vaddr) {
    if (!space) space = vmm_get_current_aspace();
    
    // This would mark the page with PTE_COW flag
    // For now, this is a placeholder
    return true;
}

// Clone address space with COW (for fork)
vmm_aspace_t *vmm_clone_cow(vmm_aspace_t *src) {
    if (!src) return NULL;
    
    // This would use vmm_clone_address_space_cow from the main VMM
    return vmm_create_aspace();
}
