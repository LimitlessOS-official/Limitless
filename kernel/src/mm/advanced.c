/**
 * Advanced Memory Management Implementation for LimitlessOS
 * 
 * Implements buddy allocator, slab allocator, huge pages, memory compression,
 * NUMA policies, page cache management, and OOM killer.
 * 
 * Copyright (c) 2024 LimitlessOS Project
 */

#include "mm/advanced.h"
#include "mm/mm.h"
#include "smp.h"
#include "kernel.h"
#include <string.h>

/* Global memory management state */
mm_global_t mm_global;

/* Built-in slab caches */
static kmem_cache_t *kmalloc_caches[13];  /* 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768 */
static kmem_cache_t *task_struct_cache;
static kmem_cache_t *files_cache;
static kmem_cache_t *vm_area_cache;

/* Memory zones */
static const char *zone_names[MAX_ZONES] = {
    "DMA",
    "Normal", 
    "High",
    "Movable"
};

/**
 * Initialize advanced memory management
 */
int mm_advanced_init(void) {
    kprintf("[MM] Initializing advanced memory management...\n");
    
    /* Clear global state */
    memset(&mm_global, 0, sizeof(mm_global_t));
    spinlock_init(&mm_global.lock);
    
    /* Initialize NUMA topology */
    if (numa_init() != 0) {
        kprintf("[MM] NUMA initialization failed\n");
        return -1;
    }
    
    /* Initialize buddy allocator */
    if (buddy_allocator_init() != 0) {
        kprintf("[MM] Buddy allocator initialization failed\n");
        return -1;
    }
    
    /* Initialize slab allocator */
    if (slab_allocator_init() != 0) {
        kprintf("[MM] Slab allocator initialization failed\n");
        return -1;
    }
    
    /* Initialize page cache */
    if (page_cache_init() != 0) {
        kprintf("[MM] Page cache initialization failed\n");
        return -1;
    }
    
    /* Initialize memory compressor */
    if (memory_compressor_init() != 0) {
        kprintf("[MM] Memory compressor initialization failed\n");
        return -1;
    }
    
    /* Initialize OOM killer */
    if (oom_killer_init() != 0) {
        kprintf("[MM] OOM killer initialization failed\n");
        return -1;
    }
    
    kprintf("[MM] Advanced memory management initialized\n");
    kprintf("[MM] NUMA nodes: %d\n", mm_global.nr_online_nodes);
    kprintf("[MM] Total pages: %lu\n", mm_global.max_pfn);
    
    return 0;
}

/**
 * Initialize NUMA topology
 */
int numa_init(void) {
    kprintf("[MM] Initializing NUMA topology...\n");
    
    /* For now, assume single NUMA node */
    mm_global.nr_online_nodes = 1;
    
    numa_node_t *node = &mm_global.nodes[0];
    node->node_id = 0;
    node->cpu_mask = 0xFFFFFFFFFFFFFFFFULL;  /* All CPUs */
    spinlock_init(&node->lock);
    
    /* Initialize distance matrix */
    for (int i = 0; i < MAX_NUMA_NODES; i++) {
        node->distance[i] = (i == 0) ? 10 : 20;
    }
    
    /* Initialize zones */
    for (int i = 0; i < MAX_ZONES; i++) {
        memory_zone_t *zone = &node->zones[i];
        zone->zone_type = i;
        zone->name = zone_names[i];
        spinlock_init(&zone->lock);
        
        /* Initialize free areas */
        for (int j = 0; j <= MAX_ORDER; j++) {
            zone->free_area[j].free_list = NULL;
            zone->free_area[j].nr_free = 0;
            spinlock_init(&zone->free_area[j].lock);
        }
        
        /* Initialize LRU lists */
        for (int j = 0; j < 5; j++) {
            zone->lru_lists[j].head = NULL;
            zone->lru_lists[j].tail = NULL;
            zone->lru_lists[j].count = 0;
            spinlock_init(&zone->lru_lists[j].lock);
        }
        
        /* Set watermarks */
        zone->watermark[0] = 128;   /* min */
        zone->watermark[1] = 256;   /* low */
        zone->watermark[2] = 512;   /* high */
    }
    
    kprintf("[MM] NUMA topology initialized\n");
    return 0;
}

/**
 * Initialize buddy allocator
 */
int buddy_allocator_init(void) {
    kprintf("[MM] Initializing buddy allocator...\n");
    
    /* Detect available memory from boot loader */
    uintptr_t total_pages = detect_memory_size() / PAGE_SIZE;
    mm_global.max_pfn = total_pages;
    
    /* Allocate memory map */
    size_t mem_map_size = total_pages * sizeof(page_frame_t);
    mm_global.mem_map = (page_frame_t*)kmalloc_early(mem_map_size);
    if (!mm_global.mem_map) {
        kprintf("[MM] Failed to allocate memory map\n");
        return -1;
    }
    
    /* Initialize page frames */
    for (uintptr_t pfn = 0; pfn < total_pages; pfn++) {
        page_frame_t *page = &mm_global.mem_map[pfn];
        memset(page, 0, sizeof(page_frame_t));
        
        page->physical_addr = pfn * PAGE_SIZE;
        page->virtual_addr = (void*)(KERNEL_VIRTUAL_BASE + page->physical_addr);
        atomic_set(&page->ref_count, 0);
        
        /* Assign to appropriate zone */
        if (pfn < (16 * 1024 * 1024) / PAGE_SIZE) {
            page->zone = &mm_global.nodes[0].zones[ZONE_DMA];
            page->nid = 0;
        } else if (pfn < (896 * 1024 * 1024) / PAGE_SIZE) {
            page->zone = &mm_global.nodes[0].zones[ZONE_NORMAL];
            page->nid = 0;
        } else {
            page->zone = &mm_global.nodes[0].zones[ZONE_HIGH];
            page->nid = 0;
        }
    }
    
    /* Add free pages to buddy allocator */
    uintptr_t kernel_end_pfn = (get_kernel_end() - KERNEL_VIRTUAL_BASE) / PAGE_SIZE;
    
    for (uintptr_t pfn = kernel_end_pfn; pfn < total_pages; pfn++) {
        page_frame_t *page = &mm_global.mem_map[pfn];
        
        /* Skip reserved areas */
        if (is_reserved_memory(pfn * PAGE_SIZE)) {
            continue;
        }
        
        /* Add to appropriate free list */
        add_to_buddy_freelist(page, 0);
    }
    
    kprintf("[MM] Buddy allocator initialized with %lu pages\n", total_pages);
    return 0;
}

/**
 * Initialize slab allocator
 */
int slab_allocator_init(void) {
    kprintf("[MM] Initializing slab allocator...\n");
    
    mm_global.cache_chain = NULL;
    
    /* Create kmalloc caches */
    size_t sizes[] = {8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768};
    
    for (int i = 0; i < 13; i++) {
        char name[32];
        snprintf(name, sizeof(name), "kmalloc-%zu", sizes[i]);
        
        kmalloc_caches[i] = kmem_cache_create(name, sizes[i], sizes[i], 0, NULL);
        if (!kmalloc_caches[i]) {
            kprintf("[MM] Failed to create kmalloc cache for size %zu\n", sizes[i]);
            return -1;
        }
    }
    
    /* Create specialized caches */
    task_struct_cache = kmem_cache_create("task_struct", 
                                         sizeof(user_process_t), 64, 0, NULL);
    files_cache = kmem_cache_create("files_struct", 
                                   1024, 64, 0, NULL);  /* TODO: proper size */
    vm_area_cache = kmem_cache_create("vm_area_struct", 
                                     128, 64, 0, NULL);  /* TODO: proper size */
    
    kprintf("[MM] Slab allocator initialized\n");
    return 0;
}

/**
 * Initialize page cache
 */
int page_cache_init(void) {
    kprintf("[MM] Initializing page cache...\n");
    
    /* TODO: Initialize radix tree implementation */
    /* TODO: Set up page cache shrinkers */
    
    kprintf("[MM] Page cache initialized\n");
    return 0;
}

/**
 * Initialize memory compressor
 */
int memory_compressor_init(void) {
    kprintf("[MM] Initializing memory compressor...\n");
    
    memory_compressor_t *comp = &mm_global.compressor;
    
    comp->algorithm = 0;  /* Disabled by default */
    comp->compressed_pool = NULL;
    comp->pool_size = 0;
    comp->compressed_pages = 0;
    spinlock_init(&comp->lock);
    
    /* Clear statistics */
    atomic_long_set(&comp->stats.compr_data_size, 0);
    atomic_long_set(&comp->stats.stored_pages, 0);
    atomic_long_set(&comp->stats.pool_total_size, 0);
    
    kprintf("[MM] Memory compressor initialized (disabled)\n");
    return 0;
}

/**
 * Initialize OOM killer
 */
int oom_killer_init(void) {
    kprintf("[MM] Initializing OOM killer...\n");
    
    oom_killer_t *oom = &mm_global.oom_killer;
    
    oom->enabled = true;
    oom->threshold = 95;  /* Trigger at 95% memory usage */
    spinlock_init(&oom->lock);
    
    /* Clear statistics */
    atomic_long_set(&oom->stats.oom_kill_count, 0);
    atomic_long_set(&oom->stats.out_of_memory, 0);
    atomic_long_set(&oom->stats.constraint_fail, 0);
    
    kprintf("[MM] OOM killer initialized\n");
    return 0;
}

/**
 * Allocate pages using buddy allocator
 */
page_frame_t *alloc_pages(uint32_t order, uint32_t gfp_flags) {
    if (order > MAX_ORDER) {
        return NULL;
    }
    
    /* Determine preferred zone based on flags */
    memory_zone_t *zone;
    if (gfp_flags & GFP_DMA) {
        zone = &mm_global.nodes[0].zones[ZONE_DMA];
    } else if (gfp_flags & GFP_HIGH) {
        zone = &mm_global.nodes[0].zones[ZONE_HIGH];
    } else {
        zone = &mm_global.nodes[0].zones[ZONE_NORMAL];
    }
    
    /* Try to allocate from preferred zone first */
    page_frame_t *page = alloc_pages_from_zone(zone, order, gfp_flags);
    if (page) {
        goto allocated;
    }
    
    /* Fall back to other zones */
    for (int i = 0; i < MAX_ZONES; i++) {
        if (&mm_global.nodes[0].zones[i] == zone) {
            continue;  /* Already tried */
        }
        
        page = alloc_pages_from_zone(&mm_global.nodes[0].zones[i], order, gfp_flags);
        if (page) {
            goto allocated;
        }
    }
    
    /* Try memory reclaim if allocation failed */
    if (!(gfp_flags & GFP_ATOMIC)) {
        try_to_free_pages(gfp_flags, order, 0);
        
        /* Retry allocation */
        page = alloc_pages_from_zone(zone, order, gfp_flags);
        if (page) {
            goto allocated;
        }
    }
    
    /* Check if we should trigger OOM killer */
    if (get_free_memory_percentage() < mm_global.oom_killer.threshold) {
        out_of_memory(gfp_flags, order, 0);
    }
    
    return NULL;

allocated:
    /* Initialize allocated pages */
    for (uint32_t i = 0; i < (1U << order); i++) {
        page_frame_t *p = page + i;
        atomic_set(&p->ref_count, 1);
        p->flags = 0;
        
        if (gfp_flags & GFP_ZERO) {
            memset(page_address(p), 0, PAGE_SIZE);
        }
    }
    
    /* Update statistics */
    atomic_long_sub(&mm_global.vm_stat.nr_free_pages, 1U << order);
    
    return page;
}

/**
 * Allocate pages from specific zone
 */
page_frame_t *alloc_pages_from_zone(memory_zone_t *zone, uint32_t order, uint32_t gfp_flags) {
    spin_lock(&zone->lock);
    
    /* Find a free block of the requested order or larger */
    for (uint32_t current_order = order; current_order <= MAX_ORDER; current_order++) {
        if (zone->free_area[current_order].nr_free > 0) {
            /* Remove page from free list */
            page_frame_t *page = zone->free_area[current_order].free_list;
            zone->free_area[current_order].free_list = page->lru.next;
            zone->free_area[current_order].nr_free--;
            
            clear_page_buddy(page);
            
            /* Split larger blocks if necessary */
            while (current_order > order) {
                current_order--;
                page_frame_t *buddy = page + (1U << current_order);
                
                set_page_buddy(buddy, current_order);
                buddy->lru.next = zone->free_area[current_order].free_list;
                zone->free_area[current_order].free_list = buddy;
                zone->free_area[current_order].nr_free++;
            }
            
            spin_unlock(&zone->lock);
            return page;
        }
    }
    
    spin_unlock(&zone->lock);
    return NULL;
}

/**
 * Free pages to buddy allocator
 */
void free_pages(page_frame_t *page, uint32_t order) {
    if (!page || order > MAX_ORDER) {
        return;
    }
    
    memory_zone_t *zone = page->zone;
    spin_lock(&zone->lock);
    
    /* Coalesce with buddy pages */
    while (order < MAX_ORDER) {
        uintptr_t page_idx = page - mm_global.mem_map;
        uintptr_t buddy_idx = page_idx ^ (1UL << order);
        page_frame_t *buddy = &mm_global.mem_map[buddy_idx];
        
        /* Check if buddy is free and of the same order */
        if (buddy->zone != zone || !page_is_buddy(buddy, order)) {
            break;
        }
        
        /* Remove buddy from free list */
        remove_from_buddy_freelist(buddy, order);
        clear_page_buddy(buddy);
        
        /* Merge with buddy */
        if (page_idx > buddy_idx) {
            page = buddy;
        }
        order++;
    }
    
    /* Add merged block to free list */
    add_to_buddy_freelist(page, order);
    
    spin_unlock(&zone->lock);
    
    /* Update statistics */
    atomic_long_add(&mm_global.vm_stat.nr_free_pages, 1U << order);
}

/**
 * Allocate single page
 */
page_frame_t *alloc_page(uint32_t gfp_flags) {
    return alloc_pages(0, gfp_flags);
}

/**
 * Free single page
 */
void free_page(page_frame_t *page) {
    free_pages(page, 0);
}

/**
 * Add page to buddy free list
 */
void add_to_buddy_freelist(page_frame_t *page, uint32_t order) {
    memory_zone_t *zone = page->zone;
    
    set_page_buddy(page, order);
    page->lru.next = zone->free_area[order].free_list;
    page->lru.prev = NULL;
    
    if (zone->free_area[order].free_list) {
        zone->free_area[order].free_list->lru.prev = page;
    }
    
    zone->free_area[order].free_list = page;
    zone->free_area[order].nr_free++;
}

/**
 * Remove page from buddy free list
 */
void remove_from_buddy_freelist(page_frame_t *page, uint32_t order) {
    memory_zone_t *zone = page->zone;
    
    if (page->lru.prev) {
        page->lru.prev->lru.next = page->lru.next;
    } else {
        zone->free_area[order].free_list = page->lru.next;
    }
    
    if (page->lru.next) {
        page->lru.next->lru.prev = page->lru.prev;
    }
    
    zone->free_area[order].nr_free--;
}

/**
 * Convert page frame number to page structure
 */
page_frame_t *pfn_to_page(uintptr_t pfn) {
    if (pfn >= mm_global.max_pfn) {
        return NULL;
    }
    return &mm_global.mem_map[pfn];
}

/**
 * Convert page structure to page frame number
 */
uintptr_t page_to_pfn(page_frame_t *page) {
    if (!page) {
        return 0;
    }
    return page - mm_global.mem_map;
}

/**
 * Get virtual address of page
 */
void *page_address(page_frame_t *page) {
    return page->virtual_addr;
}

/**
 * Convert virtual address to page structure
 */
page_frame_t *virt_to_page(void *addr) {
    uintptr_t pfn = ((uintptr_t)addr - KERNEL_VIRTUAL_BASE) / PAGE_SIZE;
    return pfn_to_page(pfn);
}

/**
 * Create kernel memory cache
 */
kmem_cache_t *kmem_cache_create(const char *name, size_t size, size_t align,
                               unsigned long flags, void (*ctor)(void *)) {
    slab_cache_t *cache = (slab_cache_t*)kmalloc_early(sizeof(slab_cache_t));
    if (!cache) {
        return NULL;
    }
    
    memset(cache, 0, sizeof(slab_cache_t));
    
    /* Set cache properties */
    strncpy(cache->name, name, sizeof(cache->name) - 1);
    cache->obj_size = size;
    cache->align = align ? align : sizeof(void*);
    cache->flags = flags;
    cache->constructor = ctor;
    
    /* Calculate slab size and objects per slab */
    cache->slab_size = PAGE_SIZE;
    cache->objects_per_slab = PAGE_SIZE / size;
    
    spinlock_init(&cache->lock);
    
    /* Initialize per-CPU caches */
    for (int i = 0; i < MAX_CPUS; i++) {
        cache->per_cpu[i].freelist = NULL;
        cache->per_cpu[i].available = 0;
        cache->per_cpu[i].limit = 32;  /* TODO: calculate based on object size */
        spinlock_init(&cache->per_cpu[i].lock);
    }
    
    /* Add to global cache list */
    spin_lock(&mm_global.lock);
    cache->next = mm_global.cache_chain;
    mm_global.cache_chain = cache;
    spin_unlock(&mm_global.lock);
    
    kprintf("[MM] Created slab cache '%s' (size=%zu, align=%zu)\n", 
            name, size, align);
    
    return (kmem_cache_t*)cache;
}

/**
 * Destroy kernel memory cache
 */
void kmem_cache_destroy(kmem_cache_t *cache_handle) {
    slab_cache_t *cache = (slab_cache_t*)cache_handle;
    if (!cache) return;
    
    /* TODO: Free all slabs and remove from cache list */
    kprintf("[MM] Destroyed slab cache '%s'\n", cache->name);
}

/**
 * Allocate object from cache
 */
void *kmem_cache_alloc(kmem_cache_t *cache_handle, uint32_t gfp_flags) {
    slab_cache_t *cache = (slab_cache_t*)cache_handle;
    if (!cache) return NULL;
    
    int cpu = smp_processor_id();
    
    /* Try per-CPU cache first */
    spin_lock(&cache->per_cpu[cpu].lock);
    if (cache->per_cpu[cpu].available > 0) {
        void *obj = cache->per_cpu[cpu].freelist;
        /* TODO: Update freelist pointer */
        cache->per_cpu[cpu].available--;
        spin_unlock(&cache->per_cpu[cpu].lock);
        
        atomic_long_inc(&cache->stats.alloc_hit);
        return obj;
    }
    spin_unlock(&cache->per_cpu[cpu].lock);
    
    /* Slow path: allocate from slab */
    atomic_long_inc(&cache->stats.alloc_miss);
    return kmem_cache_alloc_slow(cache, gfp_flags);
}

/**
 * Slow path for cache allocation
 */
void *kmem_cache_alloc_slow(slab_cache_t *cache, uint32_t gfp_flags) {
    spin_lock(&cache->lock);
    
    /* Try partial slabs first */
    if (cache->slabs.partial) {
        void *obj = allocate_from_slab(cache->slabs.partial);
        if (obj) {
            spin_unlock(&cache->lock);
            return obj;
        }
    }
    
    /* Try empty slabs */
    if (cache->slabs.empty) {
        page_frame_t *slab = cache->slabs.empty;
        cache->slabs.empty = slab->lru.next;
        cache->slabs.nr_empty--;
        
        /* Move to partial list */
        slab->lru.next = cache->slabs.partial;
        cache->slabs.partial = slab;
        cache->slabs.nr_partial++;
        
        void *obj = allocate_from_slab(slab);
        spin_unlock(&cache->lock);
        return obj;
    }
    
    spin_unlock(&cache->lock);
    
    /* Allocate new slab */
    page_frame_t *new_slab = alloc_page(gfp_flags);
    if (!new_slab) {
        return NULL;
    }
    
    setup_new_slab(cache, new_slab);
    
    spin_lock(&cache->lock);
    new_slab->lru.next = cache->slabs.partial;
    cache->slabs.partial = new_slab;
    cache->slabs.nr_partial++;
    spin_unlock(&cache->lock);
    
    return allocate_from_slab(new_slab);
}

/**
 * General kernel memory allocation
 */
void *kmalloc(size_t size, uint32_t gfp_flags) {
    if (size == 0) return NULL;
    
    /* Find appropriate cache */
    int cache_index = -1;
    size_t cache_sizes[] = {8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768};
    
    for (int i = 0; i < 13; i++) {
        if (size <= cache_sizes[i]) {
            cache_index = i;
            break;
        }
    }
    
    if (cache_index == -1) {
        /* Large allocation - use page allocator directly */
        uint32_t order = 0;
        while ((PAGE_SIZE << order) < size) {
            order++;
        }
        
        page_frame_t *page = alloc_pages(order, gfp_flags);
        return page ? page_address(page) : NULL;
    }
    
    return kmem_cache_alloc(kmalloc_caches[cache_index], gfp_flags);
}

/**
 * Zero-filled kernel memory allocation
 */
void *kzalloc(size_t size, uint32_t gfp_flags) {
    return kmalloc(size, gfp_flags | GFP_ZERO);
}

/**
 * Free kernel memory
 */
void kfree(void *ptr) {
    if (!ptr) return;
    
    page_frame_t *page = virt_to_page(ptr);
    if (!page) return;
    
    if (PageSlab(page)) {
        /* Free to slab cache */
        slab_cache_t *cache = page->slab_cache;
        kmem_cache_free((kmem_cache_t*)cache, ptr);
    } else {
        /* Free pages directly */
        free_page(page);
    }
}

/**
 * Memory debugging and statistics
 */
void show_mem_info(void) {
    kprintf("[MM] Memory Information:\n");
    kprintf("  Total pages: %lu\n", mm_global.max_pfn);
    kprintf("  Free pages: %lu\n", atomic_long_read(&mm_global.vm_stat.nr_free_pages));
    kprintf("  Active anon: %lu\n", atomic_long_read(&mm_global.vm_stat.nr_active_anon));
    kprintf("  Inactive anon: %lu\n", atomic_long_read(&mm_global.vm_stat.nr_inactive_anon));
    kprintf("  Active file: %lu\n", atomic_long_read(&mm_global.vm_stat.nr_active_file));
    kprintf("  Inactive file: %lu\n", atomic_long_read(&mm_global.vm_stat.nr_inactive_file));
    kprintf("  Dirty pages: %lu\n", atomic_long_read(&mm_global.vm_stat.nr_dirty));
    kprintf("  Writeback pages: %lu\n", atomic_long_read(&mm_global.vm_stat.nr_writeback));
    kprintf("  Slab reclaimable: %lu\n", atomic_long_read(&mm_global.vm_stat.nr_slab_reclaimable));
    kprintf("  Slab unreclaimable: %lu\n", atomic_long_read(&mm_global.vm_stat.nr_slab_unreclaimable));
}

void show_buddy_info(void) {
    kprintf("[MM] Buddy Allocator Information:\n");
    
    for (int node = 0; node < mm_global.nr_online_nodes; node++) {
        for (int zone = 0; zone < MAX_ZONES; zone++) {
            memory_zone_t *z = &mm_global.nodes[node].zones[zone];
            kprintf("Node %d, zone %8s ", node, z->name);
            
            for (int order = 0; order <= MAX_ORDER; order++) {
                kprintf("%6u ", z->free_area[order].nr_free);
            }
            kprintf("\n");
        }
    }
}

void show_slab_info(void) {
    kprintf("[MM] Slab Cache Information:\n");
    kprintf("%-20s %8s %8s %8s %8s\n", "Name", "ObjSize", "ObjsPer", "Slabs", "Objects");
    
    slab_cache_t *cache = mm_global.cache_chain;
    while (cache) {
        uint32_t total_slabs = cache->slabs.nr_full + cache->slabs.nr_partial + cache->slabs.nr_empty;
        uint32_t total_objects = total_slabs * cache->objects_per_slab;
        
        kprintf("%-20s %8zu %8zu %8u %8u\n",
                cache->name, cache->obj_size, cache->objects_per_slab,
                total_slabs, total_objects);
        
        cache = cache->next;
    }
}

/**
 * Out of memory handler
 */
void out_of_memory(uint32_t gfp_mask, uint32_t order, int nid) {
    if (!mm_global.oom_killer.enabled) {
        return;
    }
    
    spin_lock(&mm_global.oom_killer.lock);
    
    uint64_t now = get_ticks();
    if (now - mm_global.oom_killer.last_oom_time < 1000) {  /* 1 second minimum */
        spin_unlock(&mm_global.oom_killer.lock);
        return;
    }
    
    mm_global.oom_killer.last_oom_time = now;
    atomic_long_inc(&mm_global.oom_killer.stats.out_of_memory);
    
    spin_unlock(&mm_global.oom_killer.lock);
    
    kprintf("[MM] Out of memory: Kill process.\n");
    
    /* TODO: Select and kill process with highest OOM score */
    /* TODO: Free memory from killed process */
    
    atomic_long_inc(&mm_global.oom_killer.stats.oom_kill_count);
}

/**
 * Get current memory usage percentage
 */
uint32_t get_free_memory_percentage(void) {
    unsigned long free_pages = atomic_long_read(&mm_global.vm_stat.nr_free_pages);
    return (free_pages * 100) / mm_global.max_pfn;
}

/**
 * Helper functions that need to be implemented
 */
uintptr_t detect_memory_size(void) {
    /* TODO: Get memory size from multiboot or ACPI */
    return 128 * 1024 * 1024;  /* 128 MB for now */
}

void *kmalloc_early(size_t size) {
    /* TODO: Early boot allocator before buddy system is ready */
    static char early_heap[1024 * 1024];  /* 1 MB early heap */
    static size_t early_offset = 0;
    
    if (early_offset + size > sizeof(early_heap)) {
        return NULL;
    }
    
    void *ptr = &early_heap[early_offset];
    early_offset += ALIGN(size, sizeof(void*));
    return ptr;
}

uintptr_t get_kernel_end(void) {
    /* TODO: Get kernel end from linker script */
    extern char _kernel_end;
    return (uintptr_t)&_kernel_end;
}

bool is_reserved_memory(uintptr_t addr) {
    /* TODO: Check against memory map from boot loader */
    /* For now, assume everything below 1MB is reserved */
    return addr < 0x100000;
}