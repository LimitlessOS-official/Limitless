#pragma once
#include "kernel.h"
#include "vfs.h"
#include "vmm.h"
/*
 * Page Cache Subsystem (Phase 1)
 * =============================================================
 * Overview
 * --------
 * A minimal file page cache providing:
 *   - O(1) (expected) lookup via hash table keyed by (vnode*, page_index)
 *   - Global LRU list for eviction ordering
 *   - Explicit dirty tracking; write-back occurs only on:
 *       * page_cache_flush_vnode(vn)
 *       * page_cache_sync_all()
 *       * sys_munmap() of a file-backed region (best-effort whole-vnode flush)
 *   - Integration paths:
 *       * VFS read/write: page-oriented buffering to reduce storage IO
 *       * mmap + demand fault: pages mapped RO first; write fault triggers
 *         page_cache_remap_writable() which marks DIRTY
 *
 * Not (yet) implemented:
 *   - Readahead / prefetch
 *   - Background write-back daemon or throttling
 *   - Range-based flush (flushes are vnode-wide or triggered by unmap)
 *   - Memory pressure driven shrinker beyond simple target eviction calls
 *   - Concurrency safety (callers currently serialized by higher layers)
 *
 * Design & Invariants
 * -------------------
 *   - Each cached page record (page_cache_page) represents exactly one file page.
 *   - refcnt > 0 while mapped or held by callers after page_cache_get(); caller
 *     must invoke page_cache_release() exactly once per successful get.
 *   - LRU ordering: pages move to tail when looked up (unless LOCKED); on load
 *     they enter tail; eviction scans from head skipping refcnt>0 or DIRTY pages.
 *   - DIRTY flag set via:
 *       * page_cache_mark_dirty()
 *       * page_cache_remap_writable() (write-fault path for mmap)
 *       * VFS write path (after modifying in-memory contents, caller may mark)
 *   - LOCKED flag transient during initial load/population to prevent concurrent
 *     consumers from racing onto an incompletely loaded page (Phase 1: minimal usage).
 *   - Physical page memory is assumed directly accessible via a direct kernel map.
 *
 * Eviction Policy
 * ---------------
 * Simple LRU: page_cache_evict_some(target) walks from list head removing clean,
 * unreferenced (refcnt==0, !DIRTY) pages until either target reached or none eligible.
 * DIRTY pages are skipped (must be flushed first) and referenced pages are skipped.
 * A future enhancement could incorporate a two-handed clock or active/inactive lists.
 *
 * Dirty Handling
 * --------------
 *   - Write faults: mmap maps pages read-only initially (even if PROT_WRITE requested).
 *     On first write attempt, fault handler calls page_cache_remap_writable(), which:
 *       1. Ensures the page is present (loads if needed)
 *       2. Remaps with PTE_WRITABLE
 *       3. Sets DIRTY flag.
 *   - VFS writes (buffered): calling code updates the page contents and invokes
 *     page_cache_mark_dirty().
 *   - Unmap flush: sys_munmap() flushes entire vnode for simplicity (range flush TBD).
 *   - Explicit flush APIs: page_cache_flush_vnode() (per file) and page_cache_sync_all().
 *
 * Public API Summary
 * ------------------
 *   Initialization:
 *     void page_cache_init(size_t max_pages);
 *   Lookup / Pin:
 *     int page_cache_get(vnode_t* vn, u64 index, page_cache_page_t** out, bool* newly_loaded);
 *     void page_cache_release(page_cache_page_t* pg);
 *   State Mutation:
 *     void page_cache_mark_dirty(page_cache_page_t* pg);
 *     int  page_cache_flush_vnode(vnode_t* vn);
 *     int  page_cache_sync_all(void);
 *     size_t page_cache_evict_some(size_t target);
 *   Mapping Helpers (mmap/fault):
 *     int page_cache_map_into(as, vn, file_off, va, prot, writable_hint);
 *     int page_cache_remap_writable(as, vn, file_off, va, prot);
 *   Debug / Introspection:
 *     page_cache_page_info_t (fields: index, pa, refcnt, flags, present)
 *     int page_cache_debug_lookup(vnode_t* vn, u64 file_off, page_cache_page_info_t* out);
 *     size_t page_cache_debug_range(vnode_t* vn, u64 file_off, u64 length,
 *              page_cache_page_info_t* out_array, size_t max_entries, u32 flags_filter);
 *
 * Usage Patterns
 * --------------
 *   VFS Read:
 *     - For each requested file span: translate offset->page_index, call page_cache_get().
 *       Copy bytes from (pa + in_page_off). Release page.
 *   VFS Write:
 *     - Acquire page via page_cache_get() (creating if needed), modify memory, mark dirty, release.
 *   mmap Fault:
 *     - Initial: page_cache_map_into(... writable=false) -> RO mapping.
 *     - Write fault: page_cache_remap_writable() sets DIRTY.
 *   Flush:
 *     - On file sync / unmap: page_cache_flush_vnode() then optionally evict clean pages.
 *   Debugging Dirty Pages:
 *     - Use page_cache_debug_range(vn, start_off, span, arr, n, PAGE_CACHE_DIRTY) to list only dirty pages.
 *
 * Concurrency & Future Work
 * -------------------------
 *   - Phase 1 assumes single-threaded or externally serialized access; no locks
 *     are present around hash/LRU operations beyond implicit calling context.
 *   - Future: add spinlocks or fine-grained per-bucket locks, plus atomic refcounts.
 *   - Planned enhancements:
 *       * Background writeback (dirty clustering + throttling)
 *       * Page range flush (offset + length specificity)
 *       * Readahead heuristics (sequential detection)
 *       * Async IO integration (submit writes without blocking)
 *       * Memory pressure callbacks (global shrinker)
 *       * Vnode refcount integration & invalidation on truncation
 *
 * Return Codes & Error Model
 * --------------------------
 *   - Functions return 0 on success; negative K_E* constants on failure.
 *   - page_cache_get(): K_EINVAL (bad args), K_ENOMEM (alloc failure)
 *   - map helpers propagate vmm_map return codes.
 *
 * Debug Flag Semantics
 * --------------------
 *   PAGE_CACHE_PRESENT: Page has valid data loaded.
 *   PAGE_CACHE_DIRTY:   Page modified and not yet flushed.
 *   PAGE_CACHE_LOCKED:  Temporary load/creation barrier.
 *
 * Example (Dirty Page Enumeration):
 *   page_cache_page_info_t buf[32];
 *   size_t n = page_cache_debug_range(vn, 0, 512*1024, buf, 32, PAGE_CACHE_DIRTY);
 *   for(size_t i=0;i<n;i++) { [inspect buf[i].index / flags] }
 *
 * This header intentionally centralizes documentation to satisfy the
 * "Documentation updates" milestone for page cache & mmap integration.
 */
#ifdef __cplusplus
extern "C" { 
#endif

typedef struct page_cache_page page_cache_page_t;

struct page_cache_page {
    page_cache_page_t* hnext;      /* hash chain */
    page_cache_page_t* lru_prev;   /* LRU list */
    page_cache_page_t* lru_next;
    vnode_t* vnode;                /* owning vnode */
    u64 index;                     /* page index in file */
    phys_addr_t pa;                /* physical backing */
    u32 refcnt;                    /* active references (pins) */
    u32 flags;                     /* PAGE_CACHE_* */
};

enum {
    PAGE_CACHE_PRESENT = 1u << 0,
    PAGE_CACHE_DIRTY   = 1u << 1,
    PAGE_CACHE_LOCKED  = 1u << 2,
};

void page_cache_init(size_t max_pages);
int  page_cache_get(vnode_t* vn, u64 index, page_cache_page_t** out_pg, bool* newly_loaded);
void page_cache_release(page_cache_page_t* pg);
void page_cache_mark_dirty(page_cache_page_t* pg);
int  page_cache_flush_vnode(vnode_t* vn);
int  page_cache_sync_all(void);
size_t page_cache_evict_some(size_t target);

/* Stats */
struct page_cache_stats { u64 lookups; u64 hits; u64 loads; u64 flushes; u64 evictions; };
const struct page_cache_stats* page_cache_get_stats(void);
/* Map a cached page at given VA. If writable==false maps read-only even if prot requests write
 * to enable later write-fault dirty marking.
 */
int page_cache_map_into(vmm_aspace_t* as, vnode_t* vn, u64 file_off, virt_addr_t va, int prot, bool writable);
int page_cache_remap_writable(vmm_aspace_t* as, vnode_t* vn, u64 file_off, virt_addr_t va, int prot);

/* Debug: snapshot info for a single cached page (no refcount changes) */
typedef struct page_cache_page_info { u64 index; phys_addr_t pa; u32 refcnt; u32 flags; int present; } page_cache_page_info_t;
int page_cache_debug_lookup(vnode_t* vn, u64 file_off, page_cache_page_info_t* out);
/* Dump up to max_entries cached pages overlapping [file_off, file_off+length). Returns count filled.
 * Pages returned are only those present in cache; holes are skipped.
 */
size_t page_cache_debug_range(vnode_t* vn, u64 file_off, u64 length, page_cache_page_info_t* out_array, size_t max_entries, u32 flags_filter);

/* Self-test: returns 0 on success, <0 on failure */
int page_cache_selftest(void);

#ifdef __cplusplus
}
#endif
