#include "kernel.h"
#include "pci.h"
#include "pci_cfg.h"

/*
 * PCI BAR helpers and MMIO mapping.
 * vmm_iomap() is provided as a weak identity-mapping fallback elsewhere.
 */

__attribute__((weak))
void* vmm_iomap(phys_addr_t pa, size_t len) {
    (void)len;
    return (void*)(uintptr_t)pa;
}

phys_addr_t pci_get_bar_phys(const pci_device_t* d, int bar_index) {
    if (bar_index < 0 || bar_index > 5) return 0;
    u16 off = (u16)(0x10 + bar_index * 4);
    u32 bar = pci_cfg_read32(d, off);
    if ((bar & 0x1) != 0) {
        /* IO-space BAR unsupported */
        return 0;
    }
    if (((bar >> 1) & 0x3) == 0x2) {
        /* 64-bit BAR */
        u32 bar_hi = pci_cfg_read32(d, off + 4);
        return ((u64)bar_hi << 32) | (bar & ~0xFu);
    } else {
        return (phys_addr_t)(bar & ~0xFu);
    }
}