
#pragma once

#include "kernel.h"

/* Optional spinlock instrumentation.
 * Enable by defining CONFIG_LOCK_INSTRUMENT prior to including kernel.h or via build flags.
 */
#ifndef CONFIG_LOCK_INSTRUMENT
#define CONFIG_LOCK_INSTRUMENT
#endif


#ifdef CONFIG_LOCK_INSTRUMENT
typedef struct lock_stats {
    u64 acquisitions;
    u64 contended;
    u64 max_spin_loops;
    u64 total_spin_loops;
} lock_stats_t;
extern lock_stats_t g_lock_stats;
static inline void lock_stat_record(spinlock_t* l, u64 loops) {
    (void)l;
    __sync_fetch_and_add(&g_lock_stats.acquisitions, 1);
    if (loops) {
        __sync_fetch_and_add(&g_lock_stats.contended, 1);
        u64 prev_max;
        do { prev_max = g_lock_stats.max_spin_loops; if (prev_max >= loops) break; } while(!__sync_bool_compare_and_swap(&g_lock_stats.max_spin_loops, prev_max, loops));
        __sync_fetch_and_add(&g_lock_stats.total_spin_loops, loops);
    }
}
static inline void spin_lock_instrumented(spinlock_t* l) {
    u64 loops = 0;
    while (__sync_lock_test_and_set(&l->v, 1)) {
        loops++;
        __asm__ __volatile__("pause");
    }
    lock_stat_record(l, loops);
}
static inline void spin_unlock_instrumented(spinlock_t* l) {
    __sync_lock_release(&l->v);
}
#undef spin_lock
#undef spin_unlock
#define spin_lock(l)   spin_lock_instrumented(l)
#define spin_unlock(l) spin_unlock_instrumented(l)
#endif

