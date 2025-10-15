/**
 * Advanced Memory Management for LimitlessOS
 * 
 * Implements buddy allocator, slab allocator, huge pages, memory compression,
 * NUMA policies, page cache management, and OOM killer.
 * 
 * Copyright (c) 2024 LimitlessOS Project
 */

#ifndef __MM_ADVANCED_H__
#define __MM_ADVANCED_H__

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "smp.h"

/* Memory allocation flags */
#define GFP_KERNEL          0x0001      /* Kernel allocation */
#define GFP_USER            0x0002      /* User allocation */
#define GFP_ATOMIC          0x0004      /* Cannot sleep */
#define GFP_NOWAIT          0x0008      /* Cannot wait */
#define GFP_DMA             0x0010      /* DMA-able memory */
#define GFP_ZERO            0x0020      /* Zero-filled memory */
#define GFP_HIGH            0x0040      /* High memory allowed */
#define GFP_MOVABLE         0x0080      /* Movable memory */
#define GFP_RECLAIMABLE     0x0100      /* Reclaimable memory */

/* Memory zones */
typedef enum {
    ZONE_DMA,               /* 0-16MB for DMA */
    ZONE_NORMAL,            /* 16MB-896MB */
    ZONE_HIGH,              /* >896MB (32-bit) */
    ZONE_MOVABLE,           /* Movable pages */
    MAX_ZONES
} memory_zone_t;

/* Buddy allocator constants */
#define MAX_ORDER           10          /* 2^10 = 1024 pages max */
#define BUDDY_MAX_SIZE      (1 << MAX_ORDER)

/* Huge page sizes */
#define HUGEPAGE_2MB        (2 * 1024 * 1024)
#define HUGEPAGE_1GB        (1024 * 1024 * 1024)

/* NUMA constants */
#define MAX_NUMA_NODES      64
#define NUMA_NO_NODE        -1

/* Page flags */
#define PG_LOCKED           0
#define PG_ERROR            1
#define PG_REFERENCED       2
#define PG_UPTODATE         3
#define PG_DIRTY            4
#define PG_LRU              5
#define PG_ACTIVE           6
#define PG_SLAB             7
#define PG_WRITEBACK        8
#define PG_RECLAIM          9
#define PG_BUDDY            10
#define PG_COMPOUND         11
#define PG_HUGE             12
#define PG_MOVABLE          13
#define PG_RESERVED         14
#define PG_PRIVATE          15

/* Forward declarations */
typedef struct page_frame page_frame_t;
typedef struct memory_zone memory_zone_t;
typedef struct numa_node numa_node_t;
typedef struct slab_cache slab_cache_t;
typedef struct kmem_cache kmem_cache_t;

/**
 * Page frame structure
 */
struct page_frame {
    /* Page flags */
    unsigned long flags;
    
    /* Reference count */
    atomic_t ref_count;
    
    /* Zone this page belongs to */
    memory_zone_t *zone;
    
    /* NUMA node */
    int nid;
    
    /* Buddy allocator order */
    uint32_t order;
    
    /* LRU list links */
    struct {
        page_frame_t *next;
        page_frame_t *prev;
    } lru;
    
    /* For slab allocator */
    union {
        slab_cache_t *slab_cache;
        void *private_data;
    };
    
    /* Virtual address */
    void *virtual_addr;
    
    /* Physical address */
    uintptr_t physical_addr;
    
    /* Mapping information */
    struct address_space *mapping;
    pgoff_t index;
};

/**
 * Memory zone structure
 */
struct memory_zone {
    /* Zone type */
    int zone_type;
    
    /* Zone name */
    const char *name;
    
    /* Physical memory range */
    uintptr_t zone_start_pfn;
    uintptr_t zone_end_pfn;
    uintptr_t spanned_pages;
    uintptr_t present_pages;
    
    /* Free page lists for buddy allocator */
    struct {
        page_frame_t *free_list;
        uint32_t nr_free;
        spinlock_t lock;
    } free_area[MAX_ORDER + 1];
    
    /* Zone statistics */
    struct {
        atomic_long_t nr_alloc_batch;
        atomic_long_t nr_pages_min;
        atomic_long_t nr_pages_low;
        atomic_long_t nr_pages_high;
        atomic_long_t nr_free_pages;
        atomic_long_t nr_active_anon;
        atomic_long_t nr_inactive_anon;
        atomic_long_t nr_active_file;
        atomic_long_t nr_inactive_file;
        atomic_long_t nr_unevictable;
        atomic_long_t nr_mlock;
        atomic_long_t nr_writeback;
        atomic_long_t nr_unstable;
        atomic_long_t nr_bounce;
        atomic_long_t nr_vmscan_write;
        atomic_long_t nr_vmscan_immediate;
    } vm_stat;
    
    /* Watermarks */
    unsigned long watermark[3];  /* min, low, high */
    
    /* Zone lock */
    spinlock_t lock;
    
    /* LRU lists */
    struct {
        page_frame_t *head;
        page_frame_t *tail;
        size_t count;
        spinlock_t lock;
    } lru_lists[5];  /* active_anon, inactive_anon, active_file, inactive_file, unevictable */
    
    /* Reclaim state */
    struct {
        unsigned long reclaim_stat[2];
        uint32_t priority;
        bool all_unreclaimable;
    } reclaim;
};

/**
 * NUMA node structure
 */
struct numa_node {
    /* Node ID */
    int node_id;
    
    /* Memory zones in this node */
    memory_zone_t zones[MAX_ZONES];
    
    /* Node statistics */
    struct {
        uintptr_t node_start_pfn;
        uintptr_t node_present_pages;
        uintptr_t node_spanned_pages;
    } node_stat;
    
    /* Distance to other nodes */
    uint8_t distance[MAX_NUMA_NODES];
    
    /* CPU mask for this node */
    uint64_t cpu_mask;
    
    /* Memory policy */
    struct {
        int policy;
        uint64_t allowed_nodes;
    } memory_policy;
    
    /* Node lock */
    spinlock_t lock;
};

/**
 * Slab cache structure
 */
struct slab_cache {
    /* Cache name */
    char name[32];
    
    /* Object size */
    size_t obj_size;
    size_t align;
    
    /* Slab size */
    size_t slab_size;
    size_t objects_per_slab;
    
    /* Constructor/destructor */
    void (*constructor)(void *obj);
    void (*destructor)(void *obj);
    
    /* Cache flags */
    unsigned long flags;
    
    /* Per-CPU caches */
    struct {
        void **freelist;
        uint32_t available;
        uint32_t limit;
        spinlock_t lock;
    } per_cpu[MAX_CPUS];
    
    /* Slab lists */
    struct {
        page_frame_t *full;
        page_frame_t *partial;
        page_frame_t *empty;
        uint32_t nr_full;
        uint32_t nr_partial;
        uint32_t nr_empty;
    } slabs;
    
    /* Statistics */
    struct {
        atomic_long_t alloc_hit;
        atomic_long_t alloc_miss;
        atomic_long_t free_hit;
        atomic_long_t free_miss;
        atomic_long_t alloc_slowpath;
        atomic_long_t free_slowpath;
    } stats;
    
    /* Cache lock */
    spinlock_t lock;
    
    /* Link in cache list */
    slab_cache_t *next;
};

/**
 * Memory compression structure
 */
typedef struct {
    /* Compression algorithm */
    int algorithm;  /* 0=none, 1=lz4, 2=zstd */
    
    /* Compressed pages */
    void *compressed_pool;
    size_t pool_size;
    size_t compressed_pages;
    
    /* Compression statistics */
    struct {
        atomic_long_t compr_data_size;
        atomic_long_t stored_pages;
        atomic_long_t pool_total_size;
        atomic_long_t duplicate_entry;
        atomic_long_t written_back_pages;
        atomic_long_t reject_compress_poor;
    } stats;
    
    /* Compression lock */
    spinlock_t lock;
} memory_compressor_t;

/**
 * Page cache structure
 */
typedef struct address_space {
    /* Radix tree of pages */
    void *page_tree;
    
    /* Address space operations */
    struct {
        int (*readpage)(struct file *file, page_frame_t *page);
        int (*writepage)(page_frame_t *page);
        int (*sync_page)(page_frame_t *page);
        void (*invalidate_page)(page_frame_t *page);
    } ops;
    
    /* Statistics */
    struct {
        atomic_long_t nrpages;
        atomic_long_t nrexceptional;
    } stats;
    
    /* Flags */
    unsigned long flags;
    
    /* Owner inode */
    void *host;
    
    /* Address space lock */
    spinlock_t tree_lock;
} address_space_t;

/**
 * OOM killer structure
 */
typedef struct {
    /* OOM killer state */
    bool enabled;
    int threshold;
    
    /* Statistics */
    struct {
        atomic_long_t oom_kill_count;
        atomic_long_t out_of_memory;
        atomic_long_t constraint_fail;
    } stats;
    
    /* Last OOM time */
    uint64_t last_oom_time;
    
    /* OOM lock */
    spinlock_t lock;
} oom_killer_t;

/**
 * Global memory management state
 */
typedef struct {
    /* NUMA topology */
    numa_node_t nodes[MAX_NUMA_NODES];
    int nr_online_nodes;
    
    /* Global page frame array */
    page_frame_t *mem_map;
    uintptr_t max_pfn;
    
    /* Slab caches */
    slab_cache_t *cache_chain;
    
    /* Memory compressor */
    memory_compressor_t compressor;
    
    /* OOM killer */
    oom_killer_t oom_killer;
    
    /* Global statistics */
    struct {
        atomic_long_t nr_free_pages;
        atomic_long_t nr_alloc_batch;
        atomic_long_t nr_inactive_anon;
        atomic_long_t nr_active_anon;
        atomic_long_t nr_inactive_file;
        atomic_long_t nr_active_file;
        atomic_long_t nr_unevictable;
        atomic_long_t nr_mlock;
        atomic_long_t nr_anon_pages;
        atomic_long_t nr_mapped;
        atomic_long_t nr_file_pages;
        atomic_long_t nr_dirty;
        atomic_long_t nr_writeback;
        atomic_long_t nr_slab_reclaimable;
        atomic_long_t nr_slab_unreclaimable;
        atomic_long_t nr_pagetable;
        atomic_long_t nr_kernel_stack;
        atomic_long_t nr_overhead;
        atomic_long_t nr_unstable;
        atomic_long_t nr_bounce;
        atomic_long_t nr_vmscan_write;
        atomic_long_t nr_vmscan_immediate;
        atomic_long_t nr_writeback_temp;
        atomic_long_t nr_isolated_anon;
        atomic_long_t nr_isolated_file;
        atomic_long_t nr_shmem;
        atomic_long_t nr_dirtied;
        atomic_long_t nr_written;
        atomic_long_t nr_anon_transparent_hugepages;
        atomic_long_t nr_free_cma;
    } vm_stat;
    
    /* Memory hotplug */
    struct {
        bool enabled;
        void (*add_memory)(uintptr_t start, size_t size);
        void (*remove_memory)(uintptr_t start, size_t size);
    } hotplug;
    
    /* Global lock */
    spinlock_t lock;
} mm_global_t;

/* Global memory management state */
extern mm_global_t mm_global;

/* Function declarations */

/* Initialization */
int mm_advanced_init(void);
int numa_init(void);
int buddy_allocator_init(void);
int slab_allocator_init(void);
int page_cache_init(void);
int memory_compressor_init(void);
int oom_killer_init(void);

/* Buddy allocator */
page_frame_t *alloc_pages(uint32_t order, uint32_t gfp_flags);
void free_pages(page_frame_t *page, uint32_t order);
page_frame_t *alloc_page(uint32_t gfp_flags);
void free_page(page_frame_t *page);

/* Page frame management */
page_frame_t *pfn_to_page(uintptr_t pfn);
uintptr_t page_to_pfn(page_frame_t *page);
void *page_address(page_frame_t *page);
page_frame_t *virt_to_page(void *addr);

/* Slab allocator */
kmem_cache_t *kmem_cache_create(const char *name, size_t size, size_t align,
                               unsigned long flags, void (*ctor)(void *));
void kmem_cache_destroy(kmem_cache_t *cache);
void *kmem_cache_alloc(kmem_cache_t *cache, uint32_t gfp_flags);
void kmem_cache_free(kmem_cache_t *cache, void *obj);

/* General kernel memory allocation */
void *kmalloc(size_t size, uint32_t gfp_flags);
void *kzalloc(size_t size, uint32_t gfp_flags);
void *krealloc(void *ptr, size_t size, uint32_t gfp_flags);
void kfree(void *ptr);

/* NUMA support */
int numa_node_id(void);
void *kmalloc_node(size_t size, uint32_t gfp_flags, int node);
page_frame_t *alloc_pages_node(int nid, uint32_t gfp_flags, uint32_t order);
void set_memory_policy(int policy, const uint64_t *nodemask);
int get_memory_policy(int *policy, uint64_t *nodemask, void *addr);

/* Huge pages */
page_frame_t *alloc_hugepage(uint32_t gfp_flags);
void free_hugepage(page_frame_t *page);
int setup_hugepage_mapping(void *vaddr, page_frame_t *page);
void remove_hugepage_mapping(void *vaddr);

/* Memory compression */
int compress_page(page_frame_t *page);
page_frame_t *decompress_page(void *compressed_data);
void enable_memory_compression(int algorithm);
void disable_memory_compression(void);

/* Page cache */
page_frame_t *find_get_page(address_space_t *mapping, pgoff_t index);
page_frame_t *find_or_create_page(address_space_t *mapping, pgoff_t index, uint32_t gfp_flags);
void add_to_page_cache(page_frame_t *page, address_space_t *mapping, pgoff_t index);
void remove_from_page_cache(page_frame_t *page);
int sync_page_range(address_space_t *mapping, loff_t pos, loff_t count);

/* Memory reclaim */
int shrink_all_memory(unsigned long nr_pages);
int try_to_free_pages(uint32_t gfp_mask, uint32_t order, int nid);
void wakeup_kswapd(memory_zone_t *zone, uint32_t order);

/* OOM killer */
void out_of_memory(uint32_t gfp_mask, uint32_t order, int nid);
bool oom_killer_disabled(void);
void enable_oom_killer(void);
void disable_oom_killer(void);

/* Memory statistics */
void show_mem_info(void);
void show_buddy_info(void);
void show_slab_info(void);
void show_numa_info(void);

/* Memory debugging */
void check_memory_corruption(void);
void dump_page(page_frame_t *page);
void dump_zone(memory_zone_t *zone);
void validate_memory_layout(void);

/* Memory policy */
#define MPOL_DEFAULT    0
#define MPOL_PREFERRED  1
#define MPOL_BIND       2
#define MPOL_INTERLEAVE 3

/* Memory flags */
#define MEMBLOCK_NONE       0x0
#define MEMBLOCK_HOTPLUG    0x1
#define MEMBLOCK_MIRROR     0x2
#define MEMBLOCK_NOMAP      0x4

/* Inline functions for fast paths */
static inline bool page_is_buddy(page_frame_t *page, uint32_t order) {
    return (page->flags & (1UL << PG_BUDDY)) && (page->order == order);
}

static inline void set_page_buddy(page_frame_t *page, uint32_t order) {
    page->flags |= (1UL << PG_BUDDY);
    page->order = order;
}

static inline void clear_page_buddy(page_frame_t *page) {
    page->flags &= ~(1UL << PG_BUDDY);
    page->order = 0;
}

static inline bool PageLocked(page_frame_t *page) {
    return page->flags & (1UL << PG_LOCKED);
}

static inline bool PageDirty(page_frame_t *page) {
    return page->flags & (1UL << PG_DIRTY);
}

static inline bool PageSlab(page_frame_t *page) {
    return page->flags & (1UL << PG_SLAB);
}

static inline bool PageHuge(page_frame_t *page) {
    return page->flags & (1UL << PG_HUGE);
}

static inline void get_page(page_frame_t *page) {
    atomic_inc(&page->ref_count);
}

static inline void put_page(page_frame_t *page) {
    if (atomic_dec_and_test(&page->ref_count)) {
        free_page(page);
    }
}

#endif /* __MM_ADVANCED_H__ */