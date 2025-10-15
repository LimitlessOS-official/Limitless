#pragma once
#include "kernel.h"

// Externs for kernel heap allocators (used by net/core.c and others)
#ifdef __cplusplus
extern "C" {
#endif
void* vmm_kmalloc(size_t size, size_t align);
void  vmm_kfree(void* ptr, size_t size);
#ifdef __cplusplus
}
#endif

/* Spinlock definition if not already defined */

/* Additional VMM constants */
/* Use kernel.h definitions for PAGE_ALIGN macros */
#ifndef PAGE_ALIGN_DOWN
#define PAGE_ALIGN_DOWN(addr) ((addr) & ~(PAGE_SIZE - 1))
#endif
#ifndef PAGE_ALIGN_UP
#define PAGE_ALIGN_UP(addr) (((addr) + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1))
#endif
#define PTE_COW (1ull << 10)  /* Custom COW bit in available field */

/* Missing definitions */
#define PHYS_TO_VIRT_DIRECT(paddr) ((vaddr_t)(paddr) + 0xFFFF800000000000ULL)
#define VIRT_TO_PHYS_DIRECT(vaddr) ((paddr_t)((vaddr) - 0xFFFF800000000000ULL))

/*
 * Virtual Memory Manager (Phase 1)
 * - Kernel heap alloc
 * - Address space create/destroy
 * - Map/unmap pages
 * - Page fault handler hook (COW/demand paging placeholders)
 */

typedef u64 pte_flags_t;

enum {
    PTE_PRESENT = 1ull << 0,
    PTE_WRITABLE = 1ull << 1,
    PTE_USER = 1ull << 2,
    PTE_WRITETHRU = 1ull << 3,
    PTE_NOCACHE = 1ull << 4,
    PTE_ACCESSED = 1ull << 5,
    PTE_DIRTY = 1ull << 6,
    PTE_HUGE = 1ull << 7,
    PTE_GLOBAL = 1ull << 8,
    PTE_NX = 1ull << 63
};

/* W^X policy: pages may be Writable OR Executable, not both.
 * Execution permission is represented by absence of NX (PTE_NX cleared).
 */
static inline pte_flags_t vmm_enforce_wx(pte_flags_t flags) {
    int writable = (flags & PTE_WRITABLE) != 0;
    int executable = (flags & PTE_NX) == 0; /* NX cleared => exec */
    if (writable && executable) {
        /* Policy: prefer dropping execute for general mappings.
           Later we may add a two-step JIT protocol (RW then RX). */
        flags |= PTE_NX;      /* ensure non-executable */
        /* keep writable */
    }
    return flags;
}

typedef struct vmm_aspace {
    void* arch_pml; /* opaque arch root page table */
    struct vmm_region* regions; /* singly-linked list of regions */
    struct vmm_page* pages; /* shadow list of mapped pages (user) */
} vmm_aspace_t;

typedef struct vmm_region {
    struct vmm_region* next;
    virt_addr_t start;
    size_t length;
    u32 flags; /* VMM_REGION_* */
    struct vmm_file_mapping* file_map; /* non-NULL if VMM_REGION_FILE */
} vmm_region_t;

enum {
    VMM_REGION_ANON   = 0x0001, /* anonymous zero-fill-on-demand */
    VMM_REGION_WRITE  = 0x0002, /* writable after allocation */
    VMM_REGION_USER   = 0x0004, /* user accessible */
    VMM_REGION_COW    = 0x0008, /* copy-on-write (implicit when forked) */
    VMM_REGION_FILE   = 0x0010, /* file-backed (page cache) */
};

/* File-backed mapping metadata (attached via region->fs_priv or extended structure) */
typedef struct vmm_file_mapping {
    struct vnode* vnode; /* referenced vnode */
    u64 file_off;        /* start offset within file */
    u64 length;          /* mapping length */
    int prot;            /* protection flags (PROT_READ/WRITE/EXEC subset) */
} vmm_file_mapping_t;

/* Shadow page descriptor for COW bookkeeping */
typedef struct vmm_page {
    struct vmm_page* next;
    virt_addr_t va;
    phys_addr_t pa;
    pte_flags_t flags;
} vmm_page_t;

int vmm_region_add(vmm_aspace_t* as, virt_addr_t start, size_t length, u32 flags);
vmm_region_t* vmm_region_find(vmm_aspace_t* as, virt_addr_t addr);

/* Early boot */
void vmm_init(const boot_info_t* bi);

/* Kernel heap */
void* vmm_kmalloc(size_t size, size_t align);
void  vmm_kfree(void* ptr, size_t size);

/* Address space */
vmm_aspace_t* vmm_create_aspace(void);
void vmm_destroy_aspace(vmm_aspace_t* as);

/* Mapping */
int vmm_map(vmm_aspace_t* as, virt_addr_t va, phys_addr_t pa, size_t size, pte_flags_t flags);
status_t vmm_map_page(vmm_aspace_t* aspace, vaddr_t vaddr, paddr_t paddr, uint32_t flags);

/* Physical memory management */
paddr_t pmm_alloc_page(void);
void pmm_free_page(paddr_t paddr);
paddr_t pmm_alloc_pages(size_t pages);
void pmm_free_pages(paddr_t paddr, size_t pages);
int vmm_unmap(vmm_aspace_t* as, virt_addr_t va, size_t size);
/* Query physical address of a mapped virtual page */
int vmm_get_physical(vmm_aspace_t* as, virt_addr_t va, phys_addr_t* out_pa);

/* Page fault */
void vmm_handle_page_fault(u64 fault_addr, u64 err_code);

/* Fork helpers */
int vmm_clone_address_space_cow(vmm_aspace_t* dst, vmm_aspace_t* src);
vmm_region_t* vmm_region_find_range(vmm_aspace_t* as, virt_addr_t start, size_t length);
status_t vmm_unmap_page(vmm_aspace_t* aspace, vaddr_t vaddr);
vmm_aspace_t* vmm_get_current_aspace(void);
size_t vmm_get_heap_usage(void);
size_t vmm_get_heap_free(void);
status_t vmm_allocate_region(vmm_aspace_t* as, vaddr_t* out_addr, size_t size, uint32_t flags);
status_t vmm_deallocate_region(vmm_aspace_t* as, vaddr_t addr, size_t size);
int vmm_protect_ro(vmm_aspace_t* as, virt_addr_t va, size_t size);

/* Switch address space (HAL) */
extern void hal_arch_switch_aspace(void* arch_pml);
