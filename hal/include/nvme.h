#pragma once
#include "kernel.h"
#include "pci.h"

/*
 * NVMe controller - Phase 5 skeleton
 * TODO: Implement admin queue setup, I/O queue creation, and NVMe read/write.
 */

#ifdef __cplusplus
extern "C" {
#endif

void nvme_init(void);

#ifdef __cplusplus
}
#endif