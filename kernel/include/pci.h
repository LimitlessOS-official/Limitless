#pragma once
#include "kernel.h"

/* PCI/PCIe discovery (config space via HAL) */

typedef struct {
    u8 bus;
    u8 slot;
    u8 func;
    u16 vendor_id;
    u16 device_id;
    u8 class_code;
    u8 subclass;
    u8 prog_if;
} pci_device_t;

typedef void (*pci_device_cb)(const pci_device_t* dev, void* user);

void pci_init(void);
void pci_enumerate(pci_device_cb cb, void* user);

/* HAL config space */
extern u32 hal_pci_cfg_read32(u8 bus, u8 slot, u8 func, u8 offset);
extern void hal_pci_cfg_write32(u8 bus, u8 slot, u8 func, u8 offset, u32 value);