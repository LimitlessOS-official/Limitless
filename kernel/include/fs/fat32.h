#pragma once
#include "kernel.h"
#include "vfs.h"

/* Minimal FAT32 reader (read-only) */

#ifdef __cplusplus
extern "C" {
#endif

int fat32_register(void);

#ifdef __cplusplus
}
#endif