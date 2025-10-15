#pragma once
#include "kernel.h"
/* Per-CPU infrastructure (Phase 1)
 * Single-core placeholder; will expand for real SMP.
 */
#ifdef __cplusplus
extern "C" {
#endif

#ifndef KERNEL_MAX_CPUS
#define KERNEL_MAX_CPUS 8
#endif

static inline u32 percpu_cpu_count(void) {
    extern u32 hal_cpu_count(void); /* weak/stub */
    u32 c = hal_cpu_count();
    if (c == 0 || c > KERNEL_MAX_CPUS) return 1;
    return c;
}

static inline u32 percpu_current_id(void) {
    extern u32 hal_cpu_id(void); /* weak/stub */
    return hal_cpu_id();
}

typedef struct percpu_sched {
    u64 ticks;          /* total ticks accounted */
    u64 context_switch; /* number of context switches */
} percpu_sched_t;

extern percpu_sched_t g_percpu_sched[KERNEL_MAX_CPUS];

#ifdef __cplusplus
}
#endif
