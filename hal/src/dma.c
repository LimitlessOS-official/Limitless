#include "kernel.h"
#include "block_hw.h"
#include "virtio_pci.h"

/*
 * Phase 6 DMA HAL for x86_64
 *
 * Provides:
 *  - dma_alloc / dma_free: physically contiguous DMA memory
 *  - dma_map: map an existing buffer for DMA (identity fallback)
 *  - dma_sync_for_device / dma_sync_for_cpu: cache/barrier stubs
 *  - dma_bounce_*: bounce buffer helpers for devices requiring contiguous DMA
 *  - hal_virt_to_phys: fallback to identity; use VMM translation if available
 *
 * Integrates with:
 *  - pmm_alloc_contiguous/pmm_free_contiguous (if provided by PMM)
 *  - vmm_iomap() to map PA -> VA (provided by HAL; identity fallback in pci_mmio.c)
 *
 * Notes:
 *  - On QEMU with identity mapping for low memory, identity VA=PA fallback works.
 *  - Replace stubs with real cache maintenance if running on non-coherent systems.
 */

/* Optional PMM and heap interfaces (weak) */
extern phys_addr_t pmm_alloc_contiguous(size_t size, size_t align) __attribute__((weak));
extern void        pmm_free_contiguous(phys_addr_t pa, size_t size) __attribute__((weak));
extern void*       kalloc_aligned(size_t size, size_t align) __attribute__((weak));
extern void        kfree(void* ptr) __attribute__((weak));
extern phys_addr_t vmm_virt_to_phys(void* va) __attribute__((weak));

static inline size_t align_up(size_t v, size_t a) {
    return (v + a - 1) & ~(a - 1);
}

/* Fallback VA->PA translation (identity if VMM not provided) */
phys_addr_t hal_virt_to_phys(void* va) {
    if (vmm_virt_to_phys) return vmm_virt_to_phys(va);
    return (phys_addr_t)(uintptr_t)va;
}

/* Allocate physically contiguous DMA memory and map to kernel VA */
int dma_alloc(size_t len, size_t align, dma_region_t* out) {
    if (!out || align == 0) return K_EINVAL;
    size_t size = align_up(len, 4096);
    align = (align < 4096) ? 4096 : align;

    k_memset(out, 0, sizeof(*out));

    /* Prefer PMM contiguous allocation */
    if (pmm_alloc_contiguous) {
        phys_addr_t pa = pmm_alloc_contiguous(size, align);
        if (!pa) return K_ENOMEM;
        void* va = vmm_iomap(pa, size); /* identity fallback OK */
        if (!va) {
            if (pmm_free_contiguous) pmm_free_contiguous(pa, size);
            return K_ENOMEM;
        }
        out->pa = pa;
        out->va = va;
        out->len = size;
        return 0;
    }

    /* Fallback: heap allocation with identity VA=PA assumption */
    if (kalloc_aligned) {
        void* va = kalloc_aligned(size, align);
        if (!va) return K_ENOMEM;
        out->va = va;
        out->pa = hal_virt_to_phys(va);
        out->len = size;
        return 0;
    }

    return K_ENOSYS;
}

void dma_free(dma_region_t* rgn) {
    if (!rgn || !rgn->len) return;
    /* Try PMM free first */
    if (pmm_free_contiguous && rgn->pa) {
        pmm_free_contiguous(rgn->pa, rgn->len);
        rgn->pa = 0;
        rgn->va = NULL;
        rgn->len = 0;
        return;
    }
    /* Fallback heap free */
    if (kfree && rgn->va) {
        kfree(rgn->va);
        rgn->pa = 0;
        rgn->va = NULL;
        rgn->len = 0;
        return;
    }
    /* Else, nothing to do for identity-mapped regions */
    rgn->pa = 0;
    rgn->va = NULL;
    rgn->len = 0;
}

/* Map an existing buffer for DMA (no allocation). Identity fallback ok. */
int dma_map(void* buf, size_t len, dma_region_t* out) {
    if (!buf || !len || !out) return K_EINVAL;
    out->va = buf;
    out->pa = hal_virt_to_phys(buf);
    out->len = len;
    return 0;
}

/* Coherency operations (x86_64 typically coherent; use barriers as safety) */
void dma_sync_for_device(const dma_region_t* rgn) {
    (void)rgn;
    /* Ensure prior writes visible to device */
#if defined(__x86_64__)
    __asm__ __volatile__("sfence" ::: "memory");
#else
    __sync_synchronize();
#endif
}

void dma_sync_for_cpu(const dma_region_t* rgn) {
    (void)rgn;
    /* Ensure device writes visible to CPU before reading */
#if defined(__x86_64__)
    __asm__ __volatile__("lfence" ::: "memory");
#else
    __sync_synchronize();
#endif
}

/* Allocate a contiguous bounce buffer page-aligned */
int dma_bounce_alloc(size_t len, dma_region_t* out) {
    return dma_alloc(align_up(len, 4096), 4096, out);
}

/* Copy to device (host->bounce) and flush */
void dma_bounce_to_device(dma_region_t* bounce, const void* src, size_t len) {
    if (!bounce || !bounce->va || !src || len > bounce->len) return;
    k_memcpy(bounce->va, src, len);
    dma_sync_for_device(bounce);
}

/* Copy from device (bounce->host) after invalidation */
void dma_bounce_from_device(const dma_region_t* bounce, void* dst, size_t len) {
    if (!bounce || !bounce->va || !dst || len > bounce->len) return;
    dma_sync_for_cpu(bounce);
    k_memcpy(dst, bounce->va, len);
}