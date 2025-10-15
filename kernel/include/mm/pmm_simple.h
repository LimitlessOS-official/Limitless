/*
 * LimitlessOS - Production Grade Physical Memory Manager Header
 */

#ifndef PMM_SIMPLE_H
#define PMM_SIMPLE_H

#include <stdint.h>

/* Initialize PMM with memory region */
void pmm_init(uint64_t mem_start, uint64_t mem_size);

/* Allocate/free pages (returns physical address) */
uint64_t pmm_alloc_page(void);
void pmm_free_page(uint64_t addr);
uint64_t pmm_alloc_pages(uint32_t order);
void pmm_free_pages(uint64_t addr, uint32_t order);

/* Get statistics */
void pmm_get_stats(uint64_t *total, uint64_t *free);

#endif /* PMM_SIMPLE_H */
