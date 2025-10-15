/*
 * Socket Buffer (sk_buff) Implementation
 * 
 * Production-grade network packet buffer management.
 * Zero-copy design with efficient memory pooling.
 * 
 * Copyright (c) 2025 LimitlessOS Project
 */

#include "net/skbuff.h"
#include "kernel.h"
#include <string.h>

/* Global sk_buff statistics */
static skb_stats_t global_skb_stats = {0};

/* Default memory pools for common packet sizes */
static skb_pool_t small_skb_pool;   /* 256 bytes */
static skb_pool_t medium_skb_pool;  /* 1500 bytes (MTU) */
static skb_pool_t large_skb_pool;   /* 9000 bytes (jumbo frames) */

/* Spinlock for pool access (placeholder for SMP) */
static uint32_t skb_pool_lock = 0;

/* ==================== Memory Pool Management ==================== */

int skb_pool_init(skb_pool_t* pool, uint32_t count, uint32_t size) {
    if (!pool || count == 0 || size == 0) {
        return -1;
    }
    
    pool->size = SKB_DATA_ALIGN(size);
    pool->total = count;
    pool->free = 0;
    pool->allocated = 0;
    pool->freed = 0;
    pool->failed = 0;
    pool->free_list = NULL;
    
    /* Pre-allocate buffers */
    for (uint32_t i = 0; i < count; i++) {
        sk_buff_t* skb = (sk_buff_t*)kmalloc(sizeof(sk_buff_t));
        if (!skb) {
            kprintf("[SKB] Pool init failed at %u/%u buffers\n", i, count);
            return -1;
        }
        
        /* Allocate data buffer */
        uint8_t* data = (uint8_t*)kmalloc(pool->size);
        if (!data) {
            kfree(skb);
            kprintf("[SKB] Pool data allocation failed\n");
            return -1;
        }
        
        memset(skb, 0, sizeof(sk_buff_t));
        skb->head = data;
        skb->data = data;
        skb->tail = data;
        skb->end = data + pool->size;
        skb->truesize = pool->size;
        
        /* Add to free list */
        skb->next = pool->free_list;
        pool->free_list = skb;
        pool->free++;
    }
    
    return 0;
}

void skb_pool_destroy(skb_pool_t* pool) {
    if (!pool) return;
    
    sk_buff_t* skb = pool->free_list;
    while (skb) {
        sk_buff_t* next = skb->next;
        if (skb->head) {
            kfree(skb->head);
        }
        kfree(skb);
        skb = next;
    }
    
    pool->free_list = NULL;
    pool->free = 0;
}

sk_buff_t* skb_pool_alloc(skb_pool_t* pool) {
    if (!pool || !pool->free_list) {
        pool->failed++;
        return NULL;
    }
    
    /* TODO: Add spinlock for SMP safety */
    sk_buff_t* skb = pool->free_list;
    pool->free_list = skb->next;
    pool->free--;
    pool->allocated++;
    
    /* Reset skb */
    skb->next = NULL;
    skb->prev = NULL;
    skb->dev = NULL;
    skb->sk = NULL;
    skb->len = 0;
    skb->data_len = 0;
    skb->users = 1;
    skb->cloned = 0;
    skb->flags = 0;
    
    /* Reset pointers */
    skb->data = skb->head;
    skb->tail = skb->head;
    skb->mac_header = NULL;
    skb->network_header = NULL;
    skb->transport_header = NULL;
    
    global_skb_stats.pool_hits++;
    
    return skb;
}

void skb_pool_free(skb_pool_t* pool, sk_buff_t* skb) {
    if (!pool || !skb) return;
    
    /* TODO: Add spinlock for SMP safety */
    skb->next = pool->free_list;
    pool->free_list = skb;
    pool->free++;
    pool->freed++;
}

/* ==================== Core Allocation Functions ==================== */

sk_buff_t* alloc_skb(uint32_t size, uint32_t priority) {
    return alloc_skb_with_headroom(size, SKB_MIN_HEADROOM, priority);
}

sk_buff_t* alloc_skb_with_headroom(uint32_t size, uint32_t headroom, uint32_t priority) {
    sk_buff_t* skb = NULL;
    uint32_t total_size = SKB_DATA_ALIGN(size + headroom);
    
    /* Try pool allocation for common sizes */
    if (total_size <= 256 && small_skb_pool.free > 0) {
        skb = skb_pool_alloc(&small_skb_pool);
    } else if (total_size <= 1500 && medium_skb_pool.free > 0) {
        skb = skb_pool_alloc(&medium_skb_pool);
    } else if (total_size <= 9000 && large_skb_pool.free > 0) {
        skb = skb_pool_alloc(&large_skb_pool);
    }
    
    /* Pool allocation failed or size doesn't match - allocate directly */
    if (!skb) {
        global_skb_stats.pool_misses++;
        
        if (total_size > SKB_MAX_ALLOC) {
            kprintf("[SKB] Allocation too large: %u bytes\n", total_size);
            global_skb_stats.oom_count++;
            return NULL;
        }
        
        skb = (sk_buff_t*)kmalloc(sizeof(sk_buff_t));
        if (!skb) {
            global_skb_stats.oom_count++;
            return NULL;
        }
        
        uint8_t* data = (uint8_t*)kmalloc(total_size);
        if (!data) {
            kfree(skb);
            global_skb_stats.oom_count++;
            return NULL;
        }
        
        memset(skb, 0, sizeof(sk_buff_t));
        skb->head = data;
        skb->data = data + headroom;
        skb->tail = data + headroom;
        skb->end = data + total_size;
        skb->truesize = total_size;
    }
    
    /* Initialize common fields */
    skb->users = 1;
    skb->cloned = 0;
    skb->len = 0;
    skb->data_len = 0;
    skb->priority = (priority & 0xF);
    
    global_skb_stats.alloc_count++;
    
    return skb;
}

void free_skb(sk_buff_t* skb) {
    if (!skb) return;
    
    /* Check if this is from a pool */
    uint32_t size = skb->truesize;
    int from_pool = 0;
    
    if (size == small_skb_pool.size) {
        skb_pool_free(&small_skb_pool, skb);
        from_pool = 1;
    } else if (size == medium_skb_pool.size) {
        skb_pool_free(&medium_skb_pool, skb);
        from_pool = 1;
    } else if (size == large_skb_pool.size) {
        skb_pool_free(&large_skb_pool, skb);
        from_pool = 1;
    }
    
    /* Not from pool - free directly */
    if (!from_pool) {
        if (skb->head) {
            kfree(skb->head);
        }
        kfree(skb);
    }
    
    global_skb_stats.free_count++;
}

void kfree_skb(sk_buff_t* skb) {
    if (!skb) return;
    
    /* Call destructor if present */
    if (skb->destructor) {
        skb->destructor(skb);
    }
    
    free_skb(skb);
}

/* ==================== Reference Counting ==================== */

sk_buff_t* skb_get(sk_buff_t* skb) {
    if (skb) {
        skb->users++;
    }
    return skb;
}

void skb_put_ref(sk_buff_t* skb) {
    if (!skb) return;
    
    if (skb->users > 0) {
        skb->users--;
        if (skb->users == 0) {
            kfree_skb(skb);
        }
    }
}

int skb_shared(const sk_buff_t* skb) {
    return skb && skb->users > 1;
}

/* ==================== Cloning and Copying ==================== */

sk_buff_t* skb_clone(sk_buff_t* skb, uint32_t priority) {
    if (!skb) return NULL;
    
    sk_buff_t* clone = (sk_buff_t*)kmalloc(sizeof(sk_buff_t));
    if (!clone) {
        global_skb_stats.oom_count++;
        return NULL;
    }
    
    /* Copy structure */
    memcpy(clone, skb, sizeof(sk_buff_t));
    
    /* Share the data buffer */
    clone->cloned = 1;
    clone->users = 1;
    skb->users++;  /* Original buffer reference */
    
    global_skb_stats.clone_count++;
    
    return clone;
}

sk_buff_t* skb_copy(const sk_buff_t* skb, uint32_t priority) {
    if (!skb) return NULL;
    
    sk_buff_t* copy = alloc_skb(skb->len + skb_headroom(skb), priority);
    if (!copy) {
        return NULL;
    }
    
    /* Reserve headroom */
    skb_reserve(copy, skb_headroom(skb));
    
    /* Copy data */
    memcpy(skb_put(copy, skb->len), skb->data, skb->len);
    
    /* Copy metadata */
    copy->protocol = skb->protocol;
    copy->pkt_type = skb->pkt_type;
    copy->priority = skb->priority;
    copy->dev = skb->dev;
    
    global_skb_stats.copy_count++;
    
    return copy;
}

sk_buff_t* pskb_copy(sk_buff_t* skb, uint32_t priority) {
    /* For now, same as full copy - can optimize later for nonlinear skbs */
    return skb_copy(skb, priority);
}

/* ==================== Data Manipulation ==================== */

uint8_t* skb_put(sk_buff_t* skb, uint32_t len) {
    uint8_t* tmp = skb->tail;
    skb->tail += len;
    skb->len += len;
    
    if (skb->tail > skb->end) {
        kprintf("[SKB] skb_put overflow! tail=%p end=%p\n", skb->tail, skb->end);
        skb->tail = skb->end;
    }
    
    return tmp;
}

uint8_t* skb_push(sk_buff_t* skb, uint32_t len) {
    skb->data -= len;
    skb->len += len;
    
    if (skb->data < skb->head) {
        kprintf("[SKB] skb_push underflow! data=%p head=%p\n", skb->data, skb->head);
        skb->data = skb->head;
    }
    
    return skb->data;
}

uint8_t* skb_pull(sk_buff_t* skb, uint32_t len) {
    if (len > skb->len) {
        len = skb->len;
    }
    
    skb->data += len;
    skb->len -= len;
    
    return skb->data;
}

void skb_reserve(sk_buff_t* skb, uint32_t len) {
    skb->data += len;
    skb->tail += len;
}

void skb_trim(sk_buff_t* skb, uint32_t len) {
    if (skb->len > len) {
        skb->len = len;
        skb->tail = skb->data + len;
    }
}

/* ==================== Header Manipulation ==================== */

void skb_reset_mac_header(sk_buff_t* skb) {
    skb->mac_header = skb->data;
}

void skb_reset_network_header(sk_buff_t* skb) {
    skb->network_header = skb->data;
}

void skb_reset_transport_header(sk_buff_t* skb) {
    skb->transport_header = skb->data;
}

void skb_set_mac_header(sk_buff_t* skb, int offset) {
    skb->mac_header = skb->data + offset;
}

void skb_set_network_header(sk_buff_t* skb, int offset) {
    skb->network_header = skb->data + offset;
}

void skb_set_transport_header(sk_buff_t* skb, int offset) {
    skb->transport_header = skb->data + offset;
}

/* ==================== Queue Management ==================== */

void skb_queue_head_init(sk_buff_head_t* list) {
    if (!list) return;
    
    list->next = (sk_buff_t*)list;
    list->prev = (sk_buff_t*)list;
    list->qlen = 0;
    list->lock = 0;
}

void skb_queue_tail(sk_buff_head_t* list, sk_buff_t* skb) {
    if (!list || !skb) return;
    
    /* TODO: Acquire spinlock for SMP */
    
    sk_buff_t* prev = list->prev;
    skb->next = (sk_buff_t*)list;
    skb->prev = prev;
    prev->next = skb;
    list->prev = skb;
    list->qlen++;
    
    /* TODO: Release spinlock */
}

void skb_queue_head(sk_buff_head_t* list, sk_buff_t* skb) {
    if (!list || !skb) return;
    
    /* TODO: Acquire spinlock */
    
    sk_buff_t* next = list->next;
    skb->next = next;
    skb->prev = (sk_buff_t*)list;
    next->prev = skb;
    list->next = skb;
    list->qlen++;
    
    /* TODO: Release spinlock */
}

sk_buff_t* skb_dequeue(sk_buff_head_t* list) {
    if (!list || list->qlen == 0) return NULL;
    
    /* TODO: Acquire spinlock */
    
    sk_buff_t* skb = list->next;
    if (skb == (sk_buff_t*)list) {
        return NULL;
    }
    
    sk_buff_t* next = skb->next;
    list->next = next;
    next->prev = (sk_buff_t*)list;
    list->qlen--;
    
    skb->next = NULL;
    skb->prev = NULL;
    
    /* TODO: Release spinlock */
    
    return skb;
}

sk_buff_t* skb_dequeue_tail(sk_buff_head_t* list) {
    if (!list || list->qlen == 0) return NULL;
    
    /* TODO: Acquire spinlock */
    
    sk_buff_t* skb = list->prev;
    if (skb == (sk_buff_t*)list) {
        return NULL;
    }
    
    sk_buff_t* prev = skb->prev;
    list->prev = prev;
    prev->next = (sk_buff_t*)list;
    list->qlen--;
    
    skb->next = NULL;
    skb->prev = NULL;
    
    /* TODO: Release spinlock */
    
    return skb;
}

sk_buff_t* skb_peek(sk_buff_head_t* list) {
    if (!list || list->qlen == 0) return NULL;
    sk_buff_t* skb = list->next;
    return (skb == (sk_buff_t*)list) ? NULL : skb;
}

sk_buff_t* skb_peek_tail(sk_buff_head_t* list) {
    if (!list || list->qlen == 0) return NULL;
    sk_buff_t* skb = list->prev;
    return (skb == (sk_buff_t*)list) ? NULL : skb;
}

void skb_queue_purge(sk_buff_head_t* list) {
    if (!list) return;
    
    sk_buff_t* skb;
    while ((skb = skb_dequeue(list)) != NULL) {
        kfree_skb(skb);
    }
}

uint32_t skb_queue_len(const sk_buff_head_t* list) {
    return list ? list->qlen : 0;
}

int skb_queue_empty(const sk_buff_head_t* list) {
    return list ? (list->qlen == 0) : 1;
}

/* ==================== Statistics and Utilities ==================== */

void skb_get_stats(skb_stats_t* stats) {
    if (stats) {
        memcpy(stats, &global_skb_stats, sizeof(skb_stats_t));
    }
}

void skb_reset_stats(void) {
    memset(&global_skb_stats, 0, sizeof(skb_stats_t));
}

void skb_dump(const sk_buff_t* skb) {
    if (!skb) {
        kprintf("[SKB] NULL skb\n");
        return;
    }
    
    kprintf("[SKB] Dump:\n");
    kprintf("  head=%p data=%p tail=%p end=%p\n", 
            skb->head, skb->data, skb->tail, skb->end);
    kprintf("  len=%u data_len=%u truesize=%u\n",
            skb->len, skb->data_len, skb->truesize);
    kprintf("  users=%u cloned=%u priority=%u\n",
            skb->users, skb->cloned, skb->priority);
    kprintf("  protocol=0x%04x pkt_type=%u\n",
            skb->protocol, skb->pkt_type);
    kprintf("  headroom=%u tailroom=%u\n",
            skb_headroom(skb), skb_tailroom(skb));
}

int skb_validate(const sk_buff_t* skb) {
    if (!skb) return -1;
    if (!skb->head) return -1;
    if (skb->data < skb->head) return -1;
    if (skb->tail < skb->data) return -1;
    if (skb->end < skb->tail) return -1;
    if (skb->len > (uint32_t)(skb->tail - skb->data)) return -1;
    return 0;
}

/* ==================== Initialization ==================== */

int skb_init(void) {
    kprintf("[SKB] Initializing socket buffer subsystem...\n");
    
    /* Initialize memory pools */
    if (skb_pool_init(&small_skb_pool, 256, 256) < 0) {
        kprintf("[SKB] Failed to initialize small pool\n");
        return -1;
    }
    
    if (skb_pool_init(&medium_skb_pool, 512, 1500) < 0) {
        kprintf("[SKB] Failed to initialize medium pool\n");
        return -1;
    }
    
    if (skb_pool_init(&large_skb_pool, 128, 9000) < 0) {
        kprintf("[SKB] Failed to initialize large pool\n");
        return -1;
    }
    
    kprintf("[SKB] Pools: small=256x256, medium=512x1500, large=128x9000\n");
    kprintf("[SKB] Socket buffer subsystem initialized\n");
    
    return 0;
}

void skb_cleanup(void) {
    kprintf("[SKB] Cleaning up socket buffer subsystem...\n");
    
    skb_pool_destroy(&small_skb_pool);
    skb_pool_destroy(&medium_skb_pool);
    skb_pool_destroy(&large_skb_pool);
    
    kprintf("[SKB] Stats: alloc=%llu free=%llu clone=%llu copy=%llu\n",
            global_skb_stats.alloc_count,
            global_skb_stats.free_count,
            global_skb_stats.clone_count,
            global_skb_stats.copy_count);
    
    kprintf("[SKB] Socket buffer subsystem cleaned up\n");
}
