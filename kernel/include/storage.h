#pragma once
#include "kernel.h"
#include "pci.h"
#include "block.h" // reuse canonical block_dev_t

/* Storage subsystem legacy interface now delegates to block layer */

/* Register a block device (wrapper around block_register) */
static inline int storage_register_block(block_dev_t* dev) { return block_register(dev); }

/* Drivers init */
void storage_init(void);

/* Enumerate devices (simple) */
typedef void (*block_enum_cb)(block_dev_t* dev, void* user);
void storage_enumerate(block_enum_cb cb, void* user);