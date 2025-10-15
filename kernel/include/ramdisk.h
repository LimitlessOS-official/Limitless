#ifndef RAMDISK_H
#define RAMDISK_H

#include "block.h"

/*
 * RAM Disk Block Device
 * Creates virtual block devices in memory for testing
 */

/* Create a RAM disk with specified name and size in MB */
block_dev_t* ramdisk_create(const char* name, size_t size_mb);

/* Destroy a RAM disk and free its memory */
void ramdisk_destroy(block_dev_t* bdev);

#endif /* RAMDISK_H */