#pragma once
#include "kernel.h"
#include "pci.h"

/*
 * PCI config access helpers (x86 Mechanism #1)
 * Used by VirtIO PCI capability discovery
 */

#ifdef __cplusplus
extern "C" {
#endif

u8  pci_cfg_read8 (const pci_device_t* d, u16 off);
u16 pci_cfg_read16(const pci_device_t* d, u16 off);
u32 pci_cfg_read32(const pci_device_t* d, u16 off);
void pci_cfg_read(const pci_device_t* d, u16 off, void* buf, u32 len);

u8 pci_cap_first(const pci_device_t* d);
u8 pci_cap_next(const pci_device_t* d, u8 off);

#ifdef __cplusplus
}
#endif