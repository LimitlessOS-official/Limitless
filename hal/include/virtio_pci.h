#pragma once
#include "kernel.h"
#include "pci.h"
#include "pci_cfg.h"
#include "block_hw.h"

/*
 * Virtio PCI helpers (Phase 6)
 * - Capability structures and common config
 * - MMIO BAR mapping helper
 * - Capability scanner declaration
 * - x86_64 barrier and relax helpers (inline)
 *
 * TODO:
 * - MSI-X and interrupt handling
 * - Packed ring support
 */

#ifdef __cplusplus
extern "C" {
#endif

#define VIRTIO_PCI_VENDOR           0x1AF4

#define VIRTIO_PCI_CAP_COMMON_CFG   1
#define VIRTIO_PCI_CAP_NOTIFY_CFG   2
#define VIRTIO_PCI_CAP_ISR_CFG      3
#define VIRTIO_PCI_CAP_DEVICE_CFG   4
#define VIRTIO_PCI_CAP_PCI_CFG      5

#pragma pack(push, 1)
typedef struct {
    u8 cap_vndr;  /* 0x09 */
    u8 cap_next;
    u8 cap_len;
    u8 cfg_type;
    u8 bar;
    u8 padding[3];
    u32 offset;
    u32 length;
} virtio_pci_cap_t;

typedef struct {
    virtio_pci_cap_t cap;
    u32 notify_off_multiplier;
} virtio_pci_notify_cap_t;

typedef struct {
    u32 device_feature_select;
    u32 device_feature;
    u32 driver_feature_select;
    u32 driver_feature;

    u16 msix_config;
    u16 num_queues;
    u8  device_status;
    u8  config_generation;

    u16 queue_select;
    u16 queue_size;
    u16 queue_msix_vector;
    u16 queue_enable;
    u16 queue_notify_off;
    u16 queue_reserved;

    u64 queue_desc;
    u64 queue_avail;
    u64 queue_used;
} virtio_pci_common_cfg_t;
#pragma pack(pop)

/* x86_64 barriers and relax (inline) */
/* mmio_wmb and mmio_rmb are declared in block_hw.h, do not redefine here. */
static inline void cpu_relax(void) {
#if defined(__x86_64__)
    __asm__ __volatile__("pause");
#else
    __asm__ __volatile__("" ::: "memory");
#endif
}

/* MMIO mapping helpers (replace vmm_iomap with uncached mapping in VMM) */
void* vmm_iomap(phys_addr_t pa, size_t len);      /* HAL-provided */
phys_addr_t pci_get_bar_phys(const pci_device_t* d, int bar_index); /* HAL-provided */

/* Map a virtio cap MMIO window */
static inline void* virtio_ioremap(const pci_device_t* dev, int bar, u32 offset, u32 length) {
    phys_addr_t base = pci_get_bar_phys(dev, bar);
    if (!base) return NULL;
    return vmm_iomap(base + offset, length);
}

/* Capability scanner (fills pointers; out_devcfg is device-specific struct, cast by caller) */
int virtio_find_caps(const pci_device_t* dev,
                     virtio_pci_common_cfg_t** out_common,
                     void** out_notify, u32* out_notify_mul,
                     void** out_devcfg);

/* MMIO accessors */
static inline void vmmio_write16(volatile void* p, u16 v){ *(volatile u16*)p = v; mmio_wmb(); }
static inline void vmmio_write32(volatile void* p, u32 v){ *(volatile u32*)p = v; mmio_wmb(); }
static inline u16  vmmio_read16(const volatile void* p){ return *(volatile const u16*)p; }
static inline u32  vmmio_read32(const volatile void* p){ return *(volatile const u32*)p; }

#ifdef __cplusplus
}
#endif