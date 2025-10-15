/*
 * Physical Memory Manager Header
 * 
 * Copyright (c) 2024 LimitlessOS Project
 */

#ifndef PMM_H
#define PMM_H

#include <stdint.h>

#define PAGE_SIZE 4096

// Page flags
#define PAGE_FLAG_DIRTY     0x01
#define PAGE_FLAG_ACCESSED  0x02
#define PAGE_FLAG_LOCKED    0x04
#define PAGE_FLAG_RESERVED  0x08

// Initialize PMM
void pmm_init(uint64_t mem_size);

// Region management
void pmm_free_region(uint64_t base, uint64_t size);
void pmm_reserve_region(uint64_t base, uint64_t size);

// Page allocation/deallocation
void *pmm_alloc_page(void);
void *pmm_alloc_pages(uint64_t count);
void pmm_free_page(void *page);
void pmm_free_pages(void *page, uint64_t count);

// Reference counting (for COW)
void pmm_ref_page(void *page);
uint32_t pmm_get_refcount(void *page);

// Page flags
void pmm_set_page_flags(void *page, uint32_t flags);
void pmm_clear_page_flags(void *page, uint32_t flags);
uint32_t pmm_get_page_flags(void *page);

// Statistics
void pmm_get_stats(uint64_t *total, uint64_t *used, uint64_t *free);
uint64_t pmm_get_total_memory(void);
uint64_t pmm_get_free_memory(void);
uint64_t pmm_get_used_memory(void);

#endif // PMM_H
