/*
 * LimitlessOS Advanced Memory Management Header
 * Production-grade memory management definitions
 */

#ifndef _LIMITLESS_MEMORY_H
#define _LIMITLESS_MEMORY_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// Memory size constants
#define PAGE_SHIFT          12
#define PAGE_SIZE          (1UL << PAGE_SHIFT)
#define PAGE_MASK          (~(PAGE_SIZE-1))
#define HPAGE_SHIFT        21
#define HPAGE_SIZE         (1UL << HPAGE_SHIFT)
#define HPAGE_MASK         (~(HPAGE_SIZE-1))

// Maximum order for buddy allocator
#define MAX_ORDER          11
#define MAX_ORDER_NR_PAGES (1 << (MAX_ORDER-1))

// Number of NUMA nodes and zones
#define MAX_NUMNODES       64
#define MAX_NR_ZONES       6
#define MAX_ZONELISTS      2

// LRU list types
enum lru_list {
    LRU_INACTIVE_ANON = 0,
    LRU_ACTIVE_ANON = 1,
    LRU_INACTIVE_FILE = 2,
    LRU_ACTIVE_FILE = 3,
    LRU_UNEVICTABLE = 4,
    NR_LRU_LISTS
};

// Migration types for anti-fragmentation
enum migratetype {
    MIGRATE_UNMOVABLE,
    MIGRATE_MOVABLE,
    MIGRATE_RECLAIMABLE,
    MIGRATE_PCPTYPES,
    MIGRATE_HIGHATOMIC = MIGRATE_PCPTYPES,
    MIGRATE_CMA,
    MIGRATE_ISOLATE,
    MIGRATE_TYPES
};

// Memory allocation flags (gfp_t)
typedef unsigned int gfp_t;

#define ___GFP_DMA              0x01u
#define ___GFP_HIGHMEM          0x02u
#define ___GFP_DMA32            0x04u
#define ___GFP_MOVABLE          0x08u
#define ___GFP_RECLAIMABLE      0x10u
#define ___GFP_HIGH             0x20u
#define ___GFP_IO               0x40u
#define ___GFP_FS               0x80u
#define ___GFP_COLD             0x100u
#define ___GFP_NOWARN           0x200u
#define ___GFP_RETRY_MAYFAIL    0x400u
#define ___GFP_NOFAIL           0x800u
#define ___GFP_NORETRY          0x1000u
#define ___GFP_MEMALLOC         0x2000u
#define ___GFP_COMP             0x4000u
#define ___GFP_ZERO             0x8000u
#define ___GFP_NOMEMALLOC       0x10000u
#define ___GFP_HARDWALL         0x20000u
#define ___GFP_THISNODE         0x40000u
#define ___GFP_ATOMIC           0x80000u
#define ___GFP_ACCOUNT          0x100000u
#define ___GFP_DIRECT_RECLAIM   0x200000u
#define ___GFP_WRITE            0x400000u
#define ___GFP_KSWAPD_RECLAIM   0x800000u

// Watermark types
enum wmark_type {
    WMARK_MIN,
    WMARK_LOW,
    WMARK_HIGH,
    NR_WMARK
};

// VM statistics items
enum vm_stat_item {
    NR_FREE_PAGES,
    NR_ALLOC_BATCH,
    NR_INACTIVE_ANON,
    NR_ACTIVE_ANON,
    NR_INACTIVE_FILE,
    NR_ACTIVE_FILE,
    NR_UNEVICTABLE,
    NR_MLOCK,
    NR_ANON_PAGES,
    NR_MAPPED,
    NR_FILE_PAGES,
    NR_DIRTY,
    NR_WRITEBACK,
    NR_SLAB_RECLAIMABLE,
    NR_SLAB_UNRECLAIMABLE,
    NR_PAGETABLE,
    NR_KERNEL_STACK,
    NR_OVERHEAD,
    NR_UNSTABLE_NFS,
    NR_BOUNCE,
    NR_VMSCAN_WRITE,
    NR_VMSCAN_IMMEDIATE,
    NR_WRITEBACK_TEMP,
    NR_ISOLATED_ANON,
    NR_ISOLATED_FILE,
    NR_SHMEM,
    NR_DIRTIED,
    NR_WRITTEN,
    NR_PAGES_SCANNED,
    WORKINGSET_REFAULT,
    WORKINGSET_ACTIVATE,
    WORKINGSET_NODERECLAIM,
    NR_ANON_TRANSPARENT_HUGEPAGES,
    NR_FREE_CMA_PAGES,
    NR_VM_ZONE_STAT_ITEMS
};

// NUMA statistics
enum vm_numa_stat_item {
    NUMA_HIT,
    NUMA_MISS,
    NUMA_FOREIGN,
    NUMA_INTERLEAVE_HIT,
    NUMA_LOCAL,
    NUMA_OTHER,
    NR_VM_NUMA_STAT_ITEMS
};

// Global VM statistics
enum vm_global_stat_item {
    NR_DIRTY_THRESHOLD,
    NR_DIRTY_BG_THRESHOLD,
    NR_VM_STAT_ITEMS
};

// Memory overcommit policies
#define OVERCOMMIT_GUESS    0
#define OVERCOMMIT_ALWAYS   1
#define OVERCOMMIT_NEVER    2

// Memory advise behaviors
#define MADV_NORMAL     0
#define MADV_RANDOM     1
#define MADV_SEQUENTIAL 2
#define MADV_WILLNEED   3
#define MADV_DONTNEED   4
#define MADV_FREE       8
#define MADV_REMOVE     9
#define MADV_DONTFORK   10
#define MADV_DOFORK     11
#define MADV_MERGEABLE  12
#define MADV_UNMERGEABLE 13
#define MADV_HUGEPAGE   14
#define MADV_NOHUGEPAGE 15
#define MADV_DONTDUMP   16
#define MADV_DODUMP     17
#define MADV_WIPEONFORK 18
#define MADV_KEEPONFORK 19
#define MADV_HWPOISON   100

// Forward declarations
struct page;
struct zone;
struct mm_struct;
struct vm_area_struct;
struct task_struct;

// Page frame number type
typedef unsigned long pfn_t;

// Page offset type
typedef unsigned long pgoff_t;

// Page protection type
typedef struct {
    unsigned long pgprot;
} pgprot_t;

// Page directory types
typedef struct { unsigned long pgd; } pgd_t;
typedef struct { unsigned long pud; } pud_t;  
typedef struct { unsigned long pmd; } pmd_t;
typedef struct { unsigned long pte; } pte_t;

// Swap entry type
typedef struct {
    unsigned long val;
} swp_entry_t;

// Memory statistics structure
struct vm_stats {
    // Basic memory info
    unsigned long total_pages;
    unsigned long free_pages;
    unsigned long active_pages;
    unsigned long inactive_pages;
    unsigned long cached_pages;
    unsigned long buffers_pages;
    
    // Swap info
    unsigned long swap_total;
    unsigned long swap_free;
    unsigned long swap_cached;
    
    // Huge pages
    unsigned long hugepages_total;
    unsigned long hugepages_free;
    unsigned long hugepages_reserved;
    
    // Compression
    unsigned long compressed_pages;
    unsigned int compression_ratio;
    
    // Memory overcommit
    unsigned long committed_as;
    unsigned int overcommit_ratio;
    
    // VM tuning
    unsigned int swappiness;
    unsigned int dirty_ratio;
    unsigned int min_free_kbytes;
    
    // NUMA info
    unsigned int nr_nodes;
    struct {
        unsigned long total_pages;
        unsigned long free_pages;
        unsigned long distance[MAX_NUMNODES];
    } numa_stats[MAX_NUMNODES];
};

// External memory management functions
extern int vmm_init(void);
extern void vmm_shutdown(void);

// Page allocation/deallocation
extern struct page *alloc_pages(gfp_t gfp_mask, unsigned int order);
extern struct page *alloc_pages_node(int nid, gfp_t gfp_mask, unsigned int order);
extern struct page *alloc_page(gfp_t gfp_mask);
extern void __free_pages(struct page *page, unsigned int order);
extern void free_pages(unsigned long addr, unsigned int order);
extern void free_page(unsigned long addr);

// Memory mapping
extern void *vmalloc(unsigned long size);
extern void *vzalloc(unsigned long size);
extern void *vmalloc_node(unsigned long size, int node);
extern void vfree(const void *addr);
extern void *vmap(struct page **pages, unsigned int count, unsigned long flags, pgprot_t prot);
extern void vunmap(const void *addr);

// Kernel memory allocation
extern void *kmalloc(size_t size, gfp_t flags);
extern void *kzalloc(size_t size, gfp_t flags);
extern void *krealloc(const void *p, size_t new_size, gfp_t flags);
extern void kfree(const void *objp);
extern size_t ksize(const void *objp);

// Memory pools
extern mempool_t *mempool_create(int min_nr, mempool_alloc_t *alloc_fn,
                                mempool_free_t *free_fn, void *pool_data);
extern void mempool_destroy(mempool_t *pool);
extern void *mempool_alloc(mempool_t *pool, gfp_t gfp_mask);
extern void mempool_free(void *element, mempool_t *pool);

// Page management
extern void get_page(struct page *page);
extern void put_page(struct page *page);
extern int page_count(struct page *page);
extern int page_mapcount(struct page *page);
extern bool page_mapped(struct page *page);
extern void SetPageDirty(struct page *page);
extern void ClearPageDirty(struct page *page);
extern bool PageDirty(struct page *page);
extern void SetPageLocked(struct page *page);
extern void ClearPageLocked(struct page *page);
extern bool PageLocked(struct page *page);

// Memory mapping operations
extern int remap_pfn_range(struct vm_area_struct *vma, unsigned long addr,
                          unsigned long pfn, unsigned long size, pgprot_t prot);
extern int vm_insert_page(struct vm_area_struct *vma, unsigned long addr, struct page *page);
extern int vm_insert_pfn(struct vm_area_struct *vma, unsigned long addr, unsigned long pfn);

// Memory reclaim
extern int shrink_all_memory(unsigned long nr_pages);
extern void wakeup_kswapd(struct zone *zone, int order, enum zone_type classzone_idx);
extern bool zone_watermark_ok(struct zone *z, unsigned int order, unsigned long mark,
                             int classzone_idx, unsigned int alloc_flags);

// Memory compaction
extern int compact_zone(struct zone *zone, struct compact_control *cc);
extern void compact_pgdat(pg_data_t *pgdat, int order);
extern void wakeup_kcompactd(int nid, int order, enum zone_type classzone_idx);

// NUMA functions
extern int numa_node_id(void);
extern int page_to_nid(const struct page *page);
extern struct page *alloc_pages_current(gfp_t gfp, unsigned order);
extern void *__alloc_percpu(size_t size, size_t align);
extern void free_percpu(void __percpu *ptr);

// Memory hotplug
extern int add_memory(int nid, u64 start, u64 size);
extern int remove_memory(u64 start, u64 size);
extern void __online_page_set_limits(struct page *page);
extern void __online_page_increment_counters(struct page *page);
extern void __online_page_free(struct page *page);

// Memory compression
extern int memory_compress_init(void);
extern int compress_page(struct page *page);
extern int decompress_page(struct page *page);
extern void memory_compress_exit(void);

// Kernel same-page merging
extern int ksm_init(void);
extern void ksm_exit(void);
extern int ksm_madvise(struct vm_area_struct *vma, unsigned long start,
                      unsigned long end, int advice, unsigned long *vm_flags);

// Memory control groups
extern int mem_cgroup_charge(struct page *page, struct mm_struct *mm, gfp_t gfp_mask);
extern void mem_cgroup_uncharge(struct page *page);
extern void mem_cgroup_cancel_charge(struct page *page, struct mem_cgroup *memcg);

// Out-of-memory killer
extern void out_of_memory(struct zonelist *zonelist, gfp_t gfp_mask, int order);
extern bool oom_killer_disabled;
extern void mark_oom_victim(struct task_struct *tsk);
extern void wake_oom_reaper(struct task_struct *tsk);

// System calls
extern asmlinkage long sys_brk(unsigned long brk);
extern asmlinkage long sys_mmap(unsigned long addr, unsigned long len,
                               unsigned long prot, unsigned long flags,
                               unsigned long fd, unsigned long off);
extern asmlinkage long sys_munmap(unsigned long addr, size_t len);
extern asmlinkage long sys_mremap(unsigned long addr, unsigned long old_len,
                                 unsigned long new_len, unsigned long flags,
                                 unsigned long new_addr);
extern asmlinkage long sys_mprotect(unsigned long start, size_t len, unsigned long prot);
extern asmlinkage long sys_msync(unsigned long start, size_t len, int flags);
extern asmlinkage long sys_mlock(unsigned long start, size_t len);
extern asmlinkage long sys_munlock(unsigned long start, size_t len);
extern asmlinkage long sys_mlockall(int flags);
extern asmlinkage long sys_munlockall(void);
extern asmlinkage long sys_madvise(unsigned long start, size_t len, int behavior);
extern asmlinkage long sys_mincore(unsigned long start, size_t len, unsigned char *vec);

// Statistics and monitoring
extern void vmm_get_stats(struct vm_stats *stats);
extern unsigned long get_total_memory_mb(void);
extern unsigned long get_free_memory_mb(void);
extern void show_mem(unsigned int filter);

// Utility functions
extern unsigned long page_to_pfn(const struct page *page);
extern struct page *pfn_to_page(unsigned long pfn);
extern void *page_address(const struct page *page);
extern struct page *virt_to_page(const void *kaddr);
extern void *page_to_virt(const struct page *page);
extern unsigned long __pa(const void *x);
extern void *__va(unsigned long x);

// Memory barriers
extern void mb(void);
extern void rmb(void);
extern void wmb(void);
extern void smp_mb(void);
extern void smp_rmb(void);
extern void smp_wmb(void);

// Cache operations
extern void flush_cache_all(void);
extern void flush_cache_mm(struct mm_struct *mm);
extern void flush_cache_range(struct vm_area_struct *vma, unsigned long start, unsigned long end);
extern void flush_cache_page(struct vm_area_struct *vma, unsigned long addr, unsigned long pfn);

// TLB operations
extern void flush_tlb_all(void);
extern void flush_tlb_mm(struct mm_struct *mm);
extern void flush_tlb_range(struct vm_area_struct *vma, unsigned long start, unsigned long end);
extern void flush_tlb_page(struct vm_area_struct *vma, unsigned long addr);

// Constants
#define NUMA_NO_NODE    (-1)
#define NODE_PADDING_SIZE 64
#define ZONE_PADDING_SIZE 64
#define MIN_LRU_BATCH 16

// Error codes
#define ENOMEM 12
#define EAGAIN 11
#define EFAULT 14
#define EINVAL 22
#define ENOSPC 28

// Locking primitives (simplified)
typedef struct {
    volatile int lock;
} spinlock_t;

typedef struct {
    volatile int lock;
} seqlock_t;

// Memory controller
struct mem_cgroup;

// Device memory mapping
struct dev_pagemap;

// Page owner debugging
struct page_owner;

// Callback head for RCU
struct callback_head {
    struct callback_head *next;
    void (*func)(struct callback_head *head);
};

// Address space operations
struct address_space_operations;
struct address_space {
    struct inode *host;
    struct radix_tree_root page_tree;
    spinlock_t tree_lock;
    atomic_t i_mmap_writable;
    struct rb_root_cached i_mmap;
    struct rw_semaphore i_mmap_rwsem;
    unsigned long nrpages;
    unsigned long nrexceptional;
    pgoff_t writeback_index;
    const struct address_space_operations *a_ops;
    unsigned long flags;
    errseq_t wb_err;
    spinlock_t private_lock;
    struct list_head private_list;
    void *private_data;
} __attribute__((aligned(sizeof(long))));

// VM operations
struct vm_operations_struct {
    void (*open)(struct vm_area_struct *area);
    void (*close)(struct vm_area_struct *area);
    int (*split)(struct vm_area_struct *area, unsigned long addr);
    int (*mremap)(struct vm_area_struct *area);
    vm_fault_t (*fault)(struct vm_fault *vmf);
    vm_fault_t (*huge_fault)(struct vm_fault *vmf, enum page_entry_size pe_size);
    void (*map_pages)(struct vm_fault *vmf, pgoff_t start_pgoff, pgoff_t end_pgoff);
    unsigned long (*pagesize)(struct vm_area_struct *vma);
    vm_fault_t (*page_mkwrite)(struct vm_fault *vmf);
    vm_fault_t (*pfn_mkwrite)(struct vm_fault *vmf);
    int (*access)(struct vm_area_struct *vma, unsigned long addr, void *buf, int len, int write);
    const char *(*name)(struct vm_area_struct *vma);
    int (*set_policy)(struct vm_area_struct *vma, struct mempolicy *new);
    struct mempolicy *(*get_policy)(struct vm_area_struct *vma, unsigned long addr);
    struct page *(*find_special_page)(struct vm_area_struct *vma, unsigned long addr);
};

// Fault types
typedef __bitwise unsigned int vm_fault_t;

// Page entry sizes
enum page_entry_size {
    PE_SIZE_PTE = 0,
    PE_SIZE_PMD,
    PE_SIZE_PUD,
};

// VM fault structure
struct vm_fault {
    struct vm_area_struct *vma;
    unsigned int flags;
    gfp_t gfp_mask;
    pgoff_t pgoff;
    unsigned long address;
    pmd_t *pmd;
    pud_t *pud;
    pte_t orig_pte;
    struct page *cow_page;
    struct page *page;
    pte_t *pte;
    spinlock_t *ptl;
    pgtable_t prealloc_pte;
};

// Memory policy
struct mempolicy;

// Utility macros
#define PAGE_ALIGN(addr) ALIGN(addr, PAGE_SIZE)
#define PAGE_ALIGNED(addr) IS_ALIGNED((unsigned long)(addr), PAGE_SIZE)
#define offset_in_page(p) ((unsigned long)(p) & ~PAGE_MASK)

#define ALIGN(x, a) __ALIGN_KERNEL((x), (a))
#define __ALIGN_KERNEL(x, a) __ALIGN_KERNEL_MASK(x, (typeof(x))(a) - 1)
#define __ALIGN_KERNEL_MASK(x, mask) (((x) + (mask)) & ~(mask))

#define IS_ALIGNED(x, a) (((x) & ((typeof(x))(a) - 1)) == 0)

#define BITS_PER_LONG 64
#define BITS_TO_LONGS(nr) DIV_ROUND_UP(nr, BITS_PER_LONG)
#define DIV_ROUND_UP(n, d) (((n) + (d) - 1) / (d))

#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

#define container_of(ptr, type, member) ({ \
    const typeof(((type *)0)->member) * __mptr = (ptr); \
    (type *)((char *)__mptr - offsetof(type, member)); })

#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)

// Page flags manipulation
#define PG_waiters PG_arch_1

static inline void SetPageUptodate(struct page *page) { /* implementation */ }
static inline void ClearPageUptodate(struct page *page) { /* implementation */ }
static inline bool PageUptodate(struct page *page) { return false; /* implementation */ }

static inline void SetPageActive(struct page *page) { /* implementation */ }
static inline void ClearPageActive(struct page *page) { /* implementation */ }
static inline bool PageActive(struct page *page) { return false; /* implementation */ }

static inline void SetPageWriteback(struct page *page) { /* implementation */ }
static inline void ClearPageWriteback(struct page *page) { /* implementation */ }
static inline bool PageWriteback(struct page *page) { return false; /* implementation */ }

static inline void SetPageCompress(struct page *page) { /* implementation */ }
static inline void ClearPageCompress(struct page *page) { /* implementation */ }
static inline bool PageCompress(struct page *page) { return false; /* implementation */ }

static inline void SetPageAnon(struct page *page) { /* implementation */ }
static inline void ClearPageAnon(struct page *page) { /* implementation */ }
static inline bool PageAnon(struct page *page) { return false; /* implementation */ }

static inline void SetPageHuge(struct page *page) { /* implementation */ }
static inline void ClearPageHuge(struct page *page) { /* implementation */ }
static inline bool PageHuge(struct page *page) { return false; /* implementation */ }

// GFP flag combinations
#define GFP_KERNEL      (___GFP_RECLAIM | ___GFP_IO | ___GFP_FS)
#define GFP_ATOMIC      (___GFP_HIGH | ___GFP_ATOMIC | ___GFP_KSWAPD_RECLAIM)
#define GFP_USER        (___GFP_RECLAIM | ___GFP_IO | ___GFP_FS | ___GFP_HARDWALL)
#define GFP_HIGHUSER    (GFP_USER | ___GFP_HIGHMEM)
#define GFP_DMA         ___GFP_DMA
#define GFP_DMA32       ___GFP_DMA32

#endif /* _LIMITLESS_MEMORY_H */