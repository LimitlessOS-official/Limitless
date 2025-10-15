#ifndef MULTIBOOT2_H
#define MULTIBOOT2_H

#include "kernel_types.h"

/* Multiboot2 information structure */
struct multiboot_info {
    uint32_t total_size;
    uint32_t reserved;
    uint8_t tags[];
};

/* Multiboot tag structure */
struct multiboot_tag {
    uint32_t type;
    uint32_t size;
};

/* Function declarations */
void memory_init(struct multiboot_info* mbi);
void hardware_detect_and_init(void);

#endif /* MULTIBOOT2_H */