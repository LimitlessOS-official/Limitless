#pragma once
#include "kernel.h"

/* Minimal slab allocator with poisoning for hardening */
#define SLAB_POISON_ALLOC 0xAA
#define SLAB_POISON_FREE  0x55

typedef struct slab {
    struct slab* next;
    size_t obj_size;
    size_t obj_count;
    u8*    data;
    u8*    free_bitmap;
} slab_t;

void* slab_alloc(slab_t* slab);
void  slab_free(slab_t* slab, void* obj);
void  slab_poison(void* obj, size_t size, u8 val);

slab_t* slab_create(size_t obj_size, size_t obj_count);
void    slab_destroy(slab_t* slab);
