/**
 * Advanced Memory Management Enhancement for LimitlessOS
 * NUMA-aware VMM with compression, swap, pools, and defragmentation
 */

#ifndef ADVANCED_VMM_H
#define ADVANCED_VMM_H

#include "kernel.h"
#include "vmm.h"

/* Advanced memory management constants */
#define MAX_NUMA_NODES          32
#define MAX_MEMORY_POOLS        64
#define MAX_SWAP_DEVICES        16
#define MAX_MEMORY_ZONES        8
#define MEMORY_COMPRESSION_RATIO 4    /* 4:1 compression ratio target */
#define DEFRAG_THRESHOLD_PERCENT 75   /* Start defrag at 75% fragmentation */
#define MEMORY_POOL_MIN_SIZE    (4 * 1024 * 1024)   /* 4MB minimum pool */
#define SWAP_PAGE_SIZE          4096
#define NUMA_LOCAL_BOOST        10    /* 10% performance boost for local memory */

/* Memory zone types */
typedef enum {
    MEMORY_ZONE_DMA = 0,        /* DMA-able memory (0-16MB) */
    MEMORY_ZONE_DMA32,          /* 32-bit DMA memory (16MB-4GB) */
    MEMORY_ZONE_NORMAL,         /* Normal memory (4GB+) */
    MEMORY_ZONE_HIGHMEM,        /* High memory (> 896MB on 32-bit) */
    MEMORY_ZONE_MOVABLE,        /* Movable memory for defragmentation */
    MEMORY_ZONE_DEVICE,         /* Device memory (GPU, etc.) */
    MEMORY_ZONE_PERSISTENT,     /* Persistent memory (NVDIMM) */
    MEMORY_ZONE_COMPRESSED      /* Compressed memory zone */
} memory_zone_type_t;

/* NUMA node information */
typedef struct {
    uint32_t node_id;
    uint64_t base_address;
    uint64_t size;
    uint32_t cpu_mask;          /* CPUs associated with this node */
    
    /* Performance characteristics */
    uint32_t access_latency_ns; /* Local access latency */
    uint32_t bandwidth_mbps;    /* Memory bandwidth */
    
    /* Usage statistics */
    uint64_t total_pages;
    uint64_t free_pages;
    uint64_t cached_pages;
    uint64_t active_pages;
    uint64_t inactive_pages;
    
    /* Zone information for this node */
    struct {
        uint64_t start_pfn;     /* Page Frame Number start */
        uint64_t end_pfn;       /* Page Frame Number end */
        uint64_t free_pages;
        uint64_t watermark_low;
        uint64_t watermark_high;
    } zones[MAX_MEMORY_ZONES];
    
    spinlock_t lock;
} numa_node_t;

/* Memory pool types */
typedef enum {
    POOL_TYPE_GENERAL = 0,      /* General purpose allocation */
    POOL_TYPE_DMA,              /* DMA allocations */
    POOL_TYPE_KERNEL,           /* Kernel data structures */
    POOL_TYPE_USER,             /* User space allocations */
    POOL_TYPE_CACHE,            /* Page cache */
    POOL_TYPE_SLAB,             /* Slab allocator */
    POOL_TYPE_HUGE_PAGE,        /* Huge page allocations */
    POOL_TYPE_COMPRESSED        /* Compressed memory pool */
} memory_pool_type_t;

/* Memory pool */
typedef struct {
    uint32_t pool_id;
    memory_pool_type_t type;
    char name[32];
    
    /* Pool configuration */
    uint64_t base_address;
    uint64_t size;
    uint32_t page_size;         /* 4KB, 2MB, 1GB pages */
    uint32_t numa_node;         /* Preferred NUMA node */
    
    /* Allocation tracking */
    uint64_t total_pages;
    uint64_t allocated_pages;
    uint64_t free_pages;
    
    /* Free page lists by order (buddy allocator) */
    struct {
        list_head_t free_list;
        uint32_t free_count;
    } free_area[11];           /* Orders 0-10 (4KB to 4MB) */
    
    /* Pool statistics */
    uint64_t allocation_count;
    uint64_t deallocation_count;
    uint64_t bytes_allocated;
    uint64_t peak_usage;
    uint64_t fragmentation_score;
    
    /* Compression support */
    bool compression_enabled;
    uint32_t compression_ratio;
    uint64_t compressed_pages;
    
    spinlock_t lock;
    struct memory_pool_s* next;
} memory_pool_t;

/* Swap device */
typedef struct {
    uint32_t swap_id;
    char device_path[256];
    
    /* Device characteristics */
    uint64_t total_pages;
    uint64_t used_pages;
    uint32_t page_size;
    uint32_t priority;          /* Higher priority = preferred */
    
    /* Performance metrics */
    uint32_t read_latency_us;   /* Read latency in microseconds */
    uint32_t write_latency_us;  /* Write latency in microseconds */
    uint64_t read_operations;
    uint64_t write_operations;
    uint64_t bytes_read;
    uint64_t bytes_written;
    
    /* Swap space management */
    bitmap_t* swap_map;         /* Bitmap of allocated swap slots */
    uint32_t cluster_size;      /* Pages to swap together */
    
    /* Device operations */
    status_t (*read_page)(struct swap_device_s* dev, uint64_t offset, void* buffer);
    status_t (*write_page)(struct swap_device_s* dev, uint64_t offset, const void* buffer);
    
    bool active;
    spinlock_t lock;
} swap_device_t;

/* Memory compression algorithm */
typedef enum {
    COMPRESS_NONE = 0,
    COMPRESS_LZ4,               /* Fast compression */
    COMPRESS_ZSTD,              /* Balanced compression */
    COMPRESS_LZO,               /* Ultra-fast compression */
    COMPRESS_DEFLATE            /* High compression ratio */
} compression_algorithm_t;

/* Compressed page */
typedef struct {
    uint64_t original_pfn;      /* Original page frame number */
    uint32_t compressed_size;
    compression_algorithm_t algorithm;
    void* compressed_data;
    uint64_t access_count;
    uint64_t last_access_time;
    struct compressed_page_s* next;
} compressed_page_t;

/* Memory compression context */
typedef struct {
    bool enabled;
    compression_algorithm_t default_algorithm;
    uint32_t compression_threshold; /* Compress when free memory < threshold */
    
    /* Compression statistics */
    uint64_t pages_compressed;
    uint64_t pages_decompressed;
    uint64_t bytes_saved;
    uint64_t compression_time_ns;
    uint64_t decompression_time_ns;
    
    /* Compressed page management */
    compressed_page_t* compressed_pages[1024 * 1024]; /* Hash table */
    uint32_t compressed_page_count;
    
    /* Compression pools */
    memory_pool_t* compression_pool;
    
    spinlock_t lock;
} memory_compression_t;

/* Memory defragmentation */
typedef struct {
    bool enabled;
    bool active;                /* Currently running defragmentation */
    uint32_t threshold_percent; /* Start defrag at this fragmentation level */
    
    /* Defragmentation statistics */
    uint64_t defrag_cycles;
    uint64_t pages_moved;
    uint64_t time_spent_ns;
    uint64_t fragmentation_reduced;
    
    /* Movable page tracking */
    struct {
        uint64_t pfn;
        uint32_t numa_node;
        bool movable;
    } movable_pages[1024 * 1024];
    uint32_t movable_page_count;
    
    /* Defragmentation thread */
    thread_t* defrag_thread;
    
    spinlock_t lock;
} memory_defragmentation_t;

/* Advanced VMM system state */
typedef struct {
    bool initialized;
    
    /* NUMA topology */
    numa_node_t numa_nodes[MAX_NUMA_NODES];
    uint32_t numa_node_count;
    uint32_t current_node;      /* Current CPU's NUMA node */
    
    /* Memory pools */
    memory_pool_t* pools[MAX_MEMORY_POOLS];
    uint32_t pool_count;
    memory_pool_t* default_pool;
    
    /* Swap management */
    swap_device_t swap_devices[MAX_SWAP_DEVICES];
    uint32_t swap_device_count;
    uint64_t total_swap_pages;
    uint64_t used_swap_pages;
    
    /* Memory compression */
    memory_compression_t compression;
    
    /* Memory defragmentation */
    memory_defragmentation_t defrag;
    
    /* Global statistics */
    struct {
        uint64_t total_memory;
        uint64_t free_memory;
        uint64_t cached_memory;
        uint64_t buffer_memory;
        uint64_t swap_total;
        uint64_t swap_free;
        uint32_t fragmentation_percent;
        uint64_t numa_hit_count;
        uint64_t numa_miss_count;
        uint64_t allocation_failures;
    } stats;
    
    /* Performance monitoring */
    struct {
        uint64_t allocations_per_second;
        uint64_t deallocations_per_second;
        uint64_t page_faults_per_second;
        uint64_t swap_in_per_second;
        uint64_t swap_out_per_second;
        uint32_t average_allocation_time_ns;
    } performance;
    
    spinlock_t system_lock;
} advanced_vmm_t;

/* Memory allocation hints */
typedef struct {
    uint32_t preferred_numa_node;
    memory_zone_type_t zone_type;
    uint32_t alignment;
    bool allow_compression;
    bool allow_swap;
    bool movable;
    uint32_t priority;
} memory_alloc_hints_t;

/* Memory statistics for monitoring */
typedef struct {
    /* Overall memory usage */
    uint64_t total_physical_memory;
    uint64_t available_memory;
    uint64_t used_memory;
    uint64_t cached_memory;
    uint64_t buffer_memory;
    
    /* NUMA statistics */
    uint32_t numa_nodes;
    uint64_t numa_local_allocations;
    uint64_t numa_remote_allocations;
    uint32_t numa_efficiency_percent;
    
    /* Pool statistics */
    uint32_t active_pools;
    uint64_t pool_memory_used;
    uint64_t pool_memory_free;
    uint32_t pool_fragmentation_percent;
    
    /* Swap statistics */
    uint64_t swap_total;
    uint64_t swap_used;
    uint64_t swap_free;
    uint64_t swap_in_pages;
    uint64_t swap_out_pages;
    
    /* Compression statistics */
    uint64_t compressed_pages;
    uint64_t compression_ratio_percent;
    uint64_t memory_saved_bytes;
    
    /* Defragmentation statistics */
    uint32_t fragmentation_percent;
    uint64_t defrag_cycles_completed;
    uint64_t pages_moved_total;
    
    /* Performance metrics */
    uint32_t allocation_success_rate_percent;
    uint64_t average_allocation_time_ns;
    uint64_t peak_memory_usage;
} advanced_memory_stats_t;

/* API Functions */

/* System initialization */
status_t advanced_vmm_init(void);
status_t advanced_vmm_shutdown(void);

/* NUMA management */
status_t numa_discover_topology(void);
status_t numa_register_node(uint32_t node_id, uint64_t base, uint64_t size, uint32_t cpu_mask);
uint32_t numa_get_current_node(void);
uint32_t numa_get_optimal_node(void* addr);
status_t numa_set_policy(uint32_t policy, uint32_t node_mask);

/* Memory pool management */
status_t memory_pool_create(const char* name, memory_pool_type_t type, 
                           uint64_t size, uint32_t numa_node, memory_pool_t** pool);
status_t memory_pool_destroy(memory_pool_t* pool);
void* memory_pool_alloc(memory_pool_t* pool, size_t size, const memory_alloc_hints_t* hints);
status_t memory_pool_free(memory_pool_t* pool, void* ptr, size_t size);
status_t memory_pool_get_stats(memory_pool_t* pool, memory_pool_stats_t* stats);

/* Advanced allocation */
void* vmm_alloc_numa(size_t size, uint32_t numa_node, uint32_t flags);
void* vmm_alloc_huge_page(size_t size, uint32_t numa_node);
void* vmm_alloc_contiguous(size_t size, uint64_t max_addr);
void* vmm_alloc_with_hints(size_t size, const memory_alloc_hints_t* hints);

/* Swap management */
status_t swap_device_add(const char* device_path, uint32_t priority);
status_t swap_device_remove(const char* device_path);
status_t swap_page_out(uint64_t pfn, swap_device_t** device, uint64_t* offset);
status_t swap_page_in(swap_device_t* device, uint64_t offset, uint64_t pfn);
status_t swap_get_stats(swap_stats_t* stats);

/* Memory compression */
status_t memory_compression_enable(compression_algorithm_t algorithm);
status_t memory_compression_disable(void);
status_t compress_page(uint64_t pfn, compressed_page_t** compressed);
status_t decompress_page(compressed_page_t* compressed, uint64_t pfn);
status_t compression_get_stats(compression_stats_t* stats);

/* Memory defragmentation */
status_t memory_defrag_enable(uint32_t threshold_percent);
status_t memory_defrag_disable(void);
status_t memory_defrag_start(void);
status_t memory_defrag_get_stats(defrag_stats_t* stats);
uint32_t memory_calculate_fragmentation(void);

/* Memory monitoring and statistics */
status_t advanced_memory_get_stats(advanced_memory_stats_t* stats);
status_t memory_analyze_usage(memory_usage_analysis_t* analysis);
status_t memory_set_watermarks(uint64_t low, uint64_t high);

/* Memory optimization */
status_t memory_optimize_allocation_policy(void);
status_t memory_balance_numa_usage(void);
status_t memory_compact_zones(void);

/* Debugging and diagnostics */
status_t memory_dump_pools(void);
status_t memory_dump_numa_stats(void);
status_t memory_validate_integrity(void);

#endif /* ADVANCED_VMM_H */