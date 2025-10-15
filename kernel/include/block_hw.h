#pragma once
#include "kernel.h"

/*
 * Block hardware helpers (Phase 6)
 * - DMA allocation/mapping helpers for block and NIC drivers
 * - Cache management and memory barriers
 * - Simple bounce-buffer facility for non-contiguous mappings (TODO: full scatter/gather)
 *
 * Assumptions:
 * - PMM provides page allocation and virt<->phys translation for kernel memory.
 * - Arch/HAL provides cache maintenance (no-op on coherent systems) and barriers.
 *
 * TODO:
 * - IOMMU support
 * - Non-contiguous user buffers (pinning + SG lists)
 * - Streaming DMA sync for bidirectional transfers
 */

#ifdef __cplusplus
extern "C" {
#endif

typedef struct dma_region {
    void*       va;   /* kernel VA */
    phys_addr_t pa;   /* physical address (bus address in absence of IOMMU) */
    size_t      len;  /* bytes */
} dma_region_t;

/* Allocate DMA-able, physically contiguous memory with given alignment (power-of-two). */
int dma_alloc(size_t len, size_t align, dma_region_t* out);

/* Free DMA region previously allocated by dma_alloc. */
void dma_free(dma_region_t* r);

/* Map an existing kernel buffer for DMA (assumes physically contiguous for len). */
int dma_map(void* va, size_t len, dma_region_t* out);

/* No-op on coherent systems; otherwise flush CPU caches before device reads from memory. */
void dma_sync_for_device(const dma_region_t* r);

/* No-op on coherent systems; otherwise invalidate CPU caches after device writes to memory. */
void dma_sync_for_cpu(const dma_region_t* r);

/* Memory barriers for device MMIO/PIO ordering */
void mmio_wmb(void);
void mmio_rmb(void);

/* Optional: simple bounce buffer for single I/O (alloc+copy) */
int  dma_bounce_alloc(size_t len, dma_region_t* out);
void dma_bounce_to_device(dma_region_t* bounce, const void* src, size_t len);
void dma_bounce_from_device(const dma_region_t* bounce, void* dst, size_t len);

#ifdef __cplusplus
}
#endif