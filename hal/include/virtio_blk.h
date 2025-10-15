#pragma once
#include "kernel.h"
#include "pci.h"

/*
 * virtio-blk - Phase 5 skeleton
 * TODO: Implement modern virtio (packed/legacy) queues and data path.
 */

#ifdef __cplusplus
extern "C" {
#endif

void virtio_blk_init(void);

#ifdef __cplusplus
}
#endif