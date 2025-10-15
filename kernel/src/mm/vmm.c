/*
 * LimitlessOS - Virtual Memory Manager (VMM)
 *
 * This file implements the virtual memory manager, responsible for creating
 * and managing address spaces (page tables) and mapping virtual to physical memory.
 */

// Canonical includes
#include "mm/mm.h"
#include "vmm.h"
#include <string.h>

// The kernel's address space
static vmm_aspace_t kernel_as;

// Helper function to get the next level of a page table
static uint64_t* get_next_level(uint64_t* table, size_t index, bool allocate) {
    if (table[index] & VMM_FLAG_PRESENT) {
        return (uint64_t*)(table[index] & ~0xFFF);
    }
    if (!allocate) {
        return NULL;
    }
    // Allocate a new page for the next level table
    paddr_t new_page_phys = pmm_alloc_page();
    if (!new_page_phys) {
        return NULL;
    }
    void* new_page_virt = (void*)(uintptr_t)new_page_phys;
    memset(new_page_virt, 0, PAGE_SIZE);
    table[index] = new_page_phys | VMM_FLAG_PRESENT | VMM_FLAG_WRITE | VMM_FLAG_USER;
    return (uint64_t*)new_page_virt;
}

/**
 * @brief Initializes the virtual memory manager.
 */
void vmm_init(const boot_info_t* bi) {
    // Create a new address space for the kernel
    kernel_as.arch_pml = (void*)(uintptr_t)pmm_alloc_page();
    if (!kernel_as.arch_pml) {
        // Panic: Out of memory
        return;
    }
    memset(kernel_as.arch_pml, 0, PAGE_SIZE);

    // Identity map the first 1GB of physical memory
    for (vaddr_t addr = 0; addr < 0x40000000; addr += PAGE_SIZE) {
        vmm_map_page(&kernel_as, addr, addr, VMM_FLAG_PRESENT | VMM_FLAG_WRITE);
    }

    // Switch to the new address space (HAL function)
    hal_arch_switch_aspace(kernel_as.arch_pml);
}

/**
 * @brief Maps a virtual page to a physical page in a given address space.
 * @param as The address space.
 * @param virt The virtual address to map.
 * @param phys The physical address to map to.
 * @param flags The page table entry flags.
 * @return true on success, false on failure.
 */
status_t vmm_map_page(vmm_aspace_t* aspace, vaddr_t vaddr, paddr_t paddr, uint32_t flags) {
    size_t pml4_idx = (vaddr >> 39) & 0x1FF;
    size_t pdpt_idx = (vaddr >> 30) & 0x1FF;
    size_t pdt_idx  = (vaddr >> 21) & 0x1FF;
    size_t pt_idx   = (vaddr >> 12) & 0x1FF;

    uint64_t* pdpt = get_next_level((uint64_t*)aspace->arch_pml, pml4_idx, true);
    if (!pdpt) return K_ERR;
    uint64_t* pdt = get_next_level(pdpt, pdpt_idx, true);
    if (!pdt) return K_ERR;
    uint64_t* pt = get_next_level(pdt, pdt_idx, true);
    if (!pt) return K_ERR;

    pt[pt_idx] = paddr | flags;
    return K_OK;
}

/**
 * @brief Switches the current address space.
 * @param as The address space to switch to.
 */
// Use HAL function for switching address space
// void vmm_switch_as(vmm_aspace_t* as) { hal_arch_switch_aspace(as->arch_pml); }

uint64_t* vmm_get_kernel_pml4() {
    return (uint64_t*)kernel_as.arch_pml;
}
