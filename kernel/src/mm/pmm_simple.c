/*
 * LimitlessOS - Production Grade Physical Memory Manager
 * Simple but robust buddy allocator implementation
 */

#include "mm/pmm_simple.h"
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

/* Simple memory operations */
static void* memset_local(void *s, int c, size_t n) {
    unsigned char *p = (unsigned char *)s;
    while (n--) *p++ = (unsigned char)c;
    return s;
}

/* Maximum order for buddy allocator (2^11 pages = 8MB) */
#define MAX_ORDER 11
#define PAGE_SHIFT 12
#define PAGE_SIZE (1UL << PAGE_SHIFT)
#define PAGE_MASK (~(PAGE_SIZE - 1))

/* Page frame structure */
typedef struct page_frame {
    struct page_frame *next;  /* Next in free list */
    uint32_t order;           /* Block order (power of 2) */
    uint32_t ref_count;       /* Reference count */
    uint32_t flags;           /* Page flags */
} page_frame_t;

/* Free list for each order */
typedef struct {
    page_frame_t *head;
    uint32_t count;
} free_list_t;

/* PMM state */
static struct {
    page_frame_t *page_frames;   /* Array of page frames */
    free_list_t free_lists[MAX_ORDER];
    uint64_t total_pages;
    uint64_t free_pages;
    uint64_t start_addr;
    uint32_t initialized;
} pmm_state;

/* Helper: Get page frame number from physical address */
static inline uint64_t addr_to_pfn(uint64_t addr) {
    return addr >> PAGE_SHIFT;
}

/* Helper: Get physical address from page frame number */
static inline uint64_t pfn_to_addr(uint64_t pfn) {
    return pfn << PAGE_SHIFT;
}

/* Helper: Get page frame structure from pfn */
static inline page_frame_t* pfn_to_page(uint64_t pfn) {
    if (pfn >= pmm_state.total_pages) return NULL;
    return &pmm_state.page_frames[pfn];
}

/* Helper: Get pfn from page frame structure */
static inline uint64_t page_to_pfn(page_frame_t *page) {
    return (uint64_t)(page - pmm_state.page_frames);
}

/* Helper: Calculate buddy pfn */
static inline uint64_t buddy_pfn(uint64_t pfn, uint32_t order) {
    return pfn ^ (1UL << order);
}

/* Initialize PMM with memory region */
void pmm_init(uint64_t mem_start, uint64_t mem_size) {
    if (pmm_state.initialized) return;
    
    pmm_state.start_addr = mem_start & PAGE_MASK;
    pmm_state.total_pages = mem_size >> PAGE_SHIFT;
    pmm_state.free_pages = 0;
    
    /* Initialize free lists */
    for (int i = 0; i < MAX_ORDER; i++) {
        pmm_state.free_lists[i].head = NULL;
        pmm_state.free_lists[i].count = 0;
    }
    
    /* Allocate page frame array at start of memory */
    uint64_t frames_size = pmm_state.total_pages * sizeof(page_frame_t);
    frames_size = (frames_size + PAGE_SIZE - 1) & PAGE_MASK;
    pmm_state.page_frames = (page_frame_t*)pmm_state.start_addr;
    memset_local(pmm_state.page_frames, 0, frames_size);
    
    /* Mark frame array pages as used */
    uint64_t reserved_pages = frames_size >> PAGE_SHIFT;
    
    /* Add remaining pages to buddy system */
    for (uint64_t pfn = reserved_pages; pfn < pmm_state.total_pages; pfn++) {
        page_frame_t *page = pfn_to_page(pfn);
        page->order = 0;
        page->ref_count = 0;
        page->flags = 0;
        page->next = pmm_state.free_lists[0].head;
        pmm_state.free_lists[0].head = page;
        pmm_state.free_lists[0].count++;
        pmm_state.free_pages++;
    }
    
    /* Coalesce into larger blocks */
    for (uint32_t order = 0; order < MAX_ORDER - 1; order++) {
        page_frame_t *page = pmm_state.free_lists[order].head;
        page_frame_t **prev_ptr = &pmm_state.free_lists[order].head;
        
        while (page != NULL) {
            uint64_t pfn = page_to_pfn(page);
            uint64_t buddy_pfn_val = buddy_pfn(pfn, order);
            
            /* Check if buddy is also free */
            if (buddy_pfn_val < pmm_state.total_pages) {
                page_frame_t *buddy = pfn_to_page(buddy_pfn_val);
                if (buddy->ref_count == 0 && buddy->order == order) {
                    /* Remove both from current list */
                    *prev_ptr = page->next;
                    pmm_state.free_lists[order].count--;
                    
                    /* Remove buddy from list */
                    page_frame_t **buddy_prev = &pmm_state.free_lists[order].head;
                    while (*buddy_prev != NULL) {
                        if (*buddy_prev == buddy) {
                            *buddy_prev = buddy->next;
                            pmm_state.free_lists[order].count--;
                            break;
                        }
                        buddy_prev = &(*buddy_prev)->next;
                    }
                    
                    /* Add merged block to next order */
                    uint64_t lower_pfn = (pfn < buddy_pfn_val) ? pfn : buddy_pfn_val;
                    page_frame_t *merged = pfn_to_page(lower_pfn);
                    merged->order = order + 1;
                    merged->next = pmm_state.free_lists[order + 1].head;
                    pmm_state.free_lists[order + 1].head = merged;
                    pmm_state.free_lists[order + 1].count++;
                    
                    page = *prev_ptr;
                    continue;
                }
            }
            
            prev_ptr = &page->next;
            page = page->next;
        }
    }
    
    pmm_state.initialized = 1;
}

/* Allocate pages */
uint64_t pmm_alloc_pages(uint32_t order) {
    if (!pmm_state.initialized || order >= MAX_ORDER) return 0;
    
    /* Find free block of requested order or larger */
    uint32_t current_order = order;
    while (current_order < MAX_ORDER) {
        if (pmm_state.free_lists[current_order].head != NULL) break;
        current_order++;
    }
    
    if (current_order >= MAX_ORDER) return 0; /* Out of memory */
    
    /* Remove block from free list */
    page_frame_t *page = pmm_state.free_lists[current_order].head;
    pmm_state.free_lists[current_order].head = page->next;
    pmm_state.free_lists[current_order].count--;
    
    /* Split block if necessary */
    while (current_order > order) {
        current_order--;
        uint64_t pfn = page_to_pfn(page);
        uint64_t buddy_pfn_val = pfn + (1UL << current_order);
        
        if (buddy_pfn_val < pmm_state.total_pages) {
            page_frame_t *buddy = pfn_to_page(buddy_pfn_val);
            buddy->order = current_order;
            buddy->ref_count = 0;
            buddy->next = pmm_state.free_lists[current_order].head;
            pmm_state.free_lists[current_order].head = buddy;
            pmm_state.free_lists[current_order].count++;
        }
    }
    
    page->order = order;
    page->ref_count = 1;
    
    uint32_t pages_allocated = 1U << order;
    pmm_state.free_pages -= pages_allocated;
    
    return pfn_to_addr(page_to_pfn(page));
}

/* Free pages */
void pmm_free_pages(uint64_t addr, uint32_t order) {
    if (!pmm_state.initialized || order >= MAX_ORDER) return;
    
    uint64_t pfn = addr_to_pfn(addr);
    if (pfn >= pmm_state.total_pages) return;
    
    page_frame_t *page = pfn_to_page(pfn);
    if (page->ref_count == 0) return; /* Already free */
    
    page->ref_count = 0;
    page->order = order;
    
    uint32_t pages_freed = 1U << order;
    pmm_state.free_pages += pages_freed;
    
    /* Coalesce with buddy */
    while (order < MAX_ORDER - 1) {
        uint64_t buddy_pfn_val = buddy_pfn(pfn, order);
        if (buddy_pfn_val >= pmm_state.total_pages) break;
        
        page_frame_t *buddy = pfn_to_page(buddy_pfn_val);
        if (buddy->ref_count != 0 || buddy->order != order) break;
        
        /* Remove buddy from free list */
        page_frame_t **prev = &pmm_state.free_lists[order].head;
        while (*prev != NULL) {
            if (*prev == buddy) {
                *prev = buddy->next;
                pmm_state.free_lists[order].count--;
                break;
            }
            prev = &(*prev)->next;
        }
        
        /* Merge into larger block */
        pfn = (pfn < buddy_pfn_val) ? pfn : buddy_pfn_val;
        page = pfn_to_page(pfn);
        order++;
        page->order = order;
    }
    
    /* Add to free list */
    page->next = pmm_state.free_lists[order].head;
    pmm_state.free_lists[order].head = page;
    pmm_state.free_lists[order].count++;
}

/* Allocate single page */
uint64_t pmm_alloc_page(void) {
    return pmm_alloc_pages(0);
}

/* Free single page */
void pmm_free_page(uint64_t addr) {
    pmm_free_pages(addr, 0);
}

/* Get statistics */
void pmm_get_stats(uint64_t *total, uint64_t *free) {
    if (total) *total = pmm_state.total_pages;
    if (free) *free = pmm_state.free_pages;
}
