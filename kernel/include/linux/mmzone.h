#ifndef _LINUX_MMZONE_H
#define _LINUX_MMZONE_H

#include <linux/list.h>
#include <linux/mm_types.h>

#define MAX_ORDER 11

struct free_area {
    struct list_head    free_list;
    unsigned long       nr_free;
};

typedef struct zone {
    spinlock_t      lock;
    const char      *name;
    unsigned long   zone_start_pfn;
    unsigned long   spanned_pages;
    unsigned long   present_pages;
    unsigned long   managed_pages;
    struct free_area free_area[MAX_ORDER];
} zone_t;

typedef struct pglist_data {
    zone_t node_zones[1]; // Simplified: one zone per node
    int nr_zones;
    int node_id;
    unsigned long node_start_pfn;
    unsigned long node_spanned_pages;
    unsigned long node_present_pages;
} pglist_data_t;

typedef pglist_data_t pg_data_t;

#define ZONE_NORMAL 0

#endif