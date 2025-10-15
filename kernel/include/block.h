#pragma once
#include "kernel.h"

/*
 * Block layer (Phase 5)
 * - Unified request interface for block devices
 * - Registration for AHCI/NVMe/virtio-blk drivers
 * - Simple synchronous read/write helpers for VFS
 *
 * TODO:
 * - Implement request queues, NCQ, DMA and async I/O.
 * - Add partition parsing (MBR/GPT) and disk management.
 */

#ifdef __cplusplus
extern "C" {
#endif

typedef struct block_dev block_dev_t;

typedef struct {
    u64 lba;       /* starting LBA */
    u32 count;     /* sectors (512B logical sectors) */
    void* buf;     /* kernel virtual buffer */
} blk_io_t;

typedef struct {
    int  (*read)(block_dev_t* dev, blk_io_t* io);
    int  (*write)(block_dev_t* dev, const blk_io_t* io);
    u32  (*sector_size)(block_dev_t* dev);     /* logical sector size, default 512 */
    u64  (*capacity_sectors)(block_dev_t* dev);/* total logical sectors */
    void (*flush)(block_dev_t* dev);
} block_ops_t;

struct block_dev {
    char       name[16];
    void*      drv;    /* driver-private */
    block_ops_t ops;
    u32        index;  /* device index (e.g., 0 for sda) */
    u32        sector_sz;
    u64        sectors;
};

int  block_register(block_dev_t* dev);
int  block_count(void);
block_dev_t* block_get(int idx);
block_dev_t* block_find_by_name(const char* name);

/* Synchronous helpers (validate bounds, choose sector size) */
int  block_read(block_dev_t* dev, u64 lba, void* buf, u32 bytes);
int  block_write(block_dev_t* dev, u64 lba, const void* buf, u32 bytes);

#ifdef __cplusplus
}
#endif