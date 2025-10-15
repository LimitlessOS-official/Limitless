/*
 * VMM Extended Functions
 * Additional VMM functions for advanced memory management
 */

#include "kernel.h"
#include "vmm.h"
#include <mm/mm.h>
#include <string.h>

// Global current address space pointer (simple implementation)
static vmm_aspace_t *current_aspace = NULL;

// Simple kmalloc using pmm (temporary implementation)
static void *simple_kmalloc(size_t size) {
    // Allocate pages
    size_t pages = (size + PAGE_SIZE - 1) / PAGE_SIZE;
    if (pages == 1) {
        paddr_t paddr = pmm_alloc_page();
        return (void *)(uintptr_t)paddr;
    }
    paddr_t paddr = pmm_alloc_pages(pages);
    return (void *)(uintptr_t)paddr;
}

// Simple kfree using pmm
static void simple_kfree(void *ptr, size_t size) {
    size_t pages = (size + PAGE_SIZE - 1) / PAGE_SIZE;
    if (pages == 1) {
        pmm_free_page((paddr_t)(uintptr_t)ptr);
    } else {
        pmm_free_pages((paddr_t)(uintptr_t)ptr, pages);
    }
}

// Get current address space
vmm_aspace_t *vmm_get_current_aspace(void) {
    return current_aspace;
}

// Set current address space
void vmm_set_current_aspace(vmm_aspace_t *as) {
    current_aspace = as;
}

// Create a new address space
vmm_aspace_t *vmm_create_aspace(void) {
    // Allocate address space structure
    vmm_aspace_t *as = (vmm_aspace_t *)simple_kmalloc(sizeof(vmm_aspace_t));
    if (!as) return NULL;
    
    // Initialize it
    paddr_t pml = pmm_alloc_page();
    if (!pml) {
        simple_kfree(as, sizeof(vmm_aspace_t));
        return NULL;
    }
    
    as->arch_pml = (void *)(uintptr_t)pml;
    as->regions = NULL;
    as->pages = NULL;
    
    return as;
}

// Unmap a page (stub for now)
status_t vmm_unmap_page(vmm_aspace_t *aspace, vaddr_t vaddr) {
    // Simplified - would actually walk page tables and unmap
    return 0;
}

// Get physical address from virtual
int vmm_get_physical(vmm_aspace_t *as, vaddr_t va, paddr_t *out_pa) {
    if (!as || !out_pa) return -1;
    
    // Simplified - would walk page tables in real implementation
    *out_pa = (paddr_t)va; // Identity mapping for now
    return 0;
}

