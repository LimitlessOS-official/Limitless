#pragma once
#include "kernel.h"

/* Virtio split ring structures (Phase 6) */
#ifdef __cplusplus
extern "C" {
#endif

#define VRING_DESC_F_NEXT     1
#define VRING_DESC_F_WRITE    2
#define VRING_DESC_F_INDIRECT 4

#define VRING_AVAIL_F_NO_INTERRUPT 1
#define VRING_USED_F_NO_NOTIFY     1

#pragma pack(push, 1)
typedef struct {
    u64 addr;
    u32 len;
    u16 flags;
    u16 next;
} vring_desc;

typedef struct {
    u16 flags;
    u16 idx;
    u16 ring[];
} vring_avail;

typedef struct {
    u32 id;
    u32 len;
} vring_used_elem;

typedef struct {
    u16 flags;
    u16 idx;
    vring_used_elem ring[];
} vring_used;
#pragma pack(pop)

static inline size_t vring_desc_size(u16 qsz) { return sizeof(vring_desc) * (size_t)qsz; }
static inline size_t vring_avail_size(u16 qsz) { return sizeof(vring_avail) + sizeof(u16) * (size_t)qsz + sizeof(u16); }
static inline size_t vring_used_size(u16 qsz) { return sizeof(vring_used) + sizeof(vring_used_elem) * (size_t)qsz + sizeof(u16); }

#ifdef __cplusplus
}
#endif