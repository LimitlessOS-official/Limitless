#pragma once
#include "kernel.h"
#include "pci.h"

/*
 * AHCI (SATA) controller - Phase 5 minimal driver interface
 * TODO: Implement full HBA init, FIS setup, command engine, and DMA.
 */

#ifdef __cplusplus
extern "C" {
#endif

void ahci_init(void); /* Enumerate via PCI, attach disks to block layer */

#ifdef __cplusplus
}
#endif