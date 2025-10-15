/**
 * SMP (Symmetric Multi-Processing) Support for LimitlessOS
 * 
 * This header defines the interfaces and structures needed for multi-core
 * processor support, including AP (Application Processor) startup, per-CPU
 * data structures, and inter-processor communication.
 * 
 * Copyright (c) 2024 LimitlessOS Project
 */

#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/* Maximum number of CPUs supported by the system */
#define MAX_CPUS 256

/* CPU states during boot and runtime */
typedef enum {
    CPU_OFFLINE = 0,    /* CPU not initialized */
    CPU_BOOTING,        /* CPU is starting up */
    CPU_ONLINE,         /* CPU active and running */
    CPU_IDLE,           /* CPU idle but available */
    CPU_STOPPING,       /* CPU being shut down */
    CPU_DEAD            /* CPU stopped/failed */
} cpu_state_t;

/* CPU topology information */
typedef struct cpu_topology {
    uint32_t package_id;    /* Physical CPU package */
    uint32_t core_id;       /* Core within package */
    uint32_t thread_id;     /* Thread within core (for SMT) */
    uint32_t numa_node;     /* NUMA node ID */
    bool is_smt;            /* Simultaneous multithreading enabled */
} cpu_topology_t;

/* Per-CPU data structure */
typedef struct cpu_info {
    /* CPU identification */
    uint32_t cpu_id;            /* Logical CPU ID (0-based) */
    uint32_t apic_id;           /* APIC ID from hardware */
    cpu_state_t state;          /* Current CPU state */
    cpu_topology_t topology;    /* CPU topology info */
    
    /* CPU-specific stacks */
    void* kernel_stack;         /* Kernel stack for this CPU */
    void* interrupt_stack;      /* Interrupt stack */
    void* exception_stack;      /* Exception stack */
    
    /* Scheduler data */
    void* current_task;         /* Currently running task */
    void* idle_task;            /* Idle task for this CPU */
    void* run_queue;            /* Per-CPU run queue */
    uint64_t load_weight;       /* Load balancing weight */
    uint32_t nr_running;        /* Number of runnable tasks */
    
    /* Timing and statistics */
    uint64_t tsc_freq;          /* TSC frequency */
    uint64_t boot_time;         /* When this CPU came online */
    uint64_t idle_time;         /* Time spent in idle */
    uint64_t irq_time;          /* Time spent in interrupts */
    
    /* Memory management */
    void* page_dir;             /* Per-CPU page directory cache */
    void* tlb_flush_mask;       /* TLB flush coordination */
    
    /* IPI (Inter-Processor Interrupt) handling */
    volatile uint32_t ipi_pending;   /* Pending IPI types */
    uint32_t ipi_count[16];          /* IPI counters by type */
    
    /* Power management */
    uint32_t c_state;           /* Current C-state */
    uint32_t p_state;           /* Current P-state */
    bool halt_requested;        /* CPU should halt */
    
    /* Cache information */
    uint32_t cache_line_size;   /* L1 cache line size */
    uint32_t l1_cache_size;     /* L1 cache size */
    uint32_t l2_cache_size;     /* L2 cache size */
    uint32_t l3_cache_size;     /* L3 cache size */
    
    /* Performance monitoring */
    uint64_t context_switches;   /* Number of context switches */
    uint64_t interrupts;         /* Interrupts handled */
    uint64_t syscalls;           /* System calls processed */
    
    /* Padding to ensure cache line alignment */
    char padding[64 - (sizeof(uint64_t) * 3) % 64];
} __attribute__((aligned(64))) cpu_info_t;

/* IPI (Inter-Processor Interrupt) types */
#define IPI_RESCHEDULE      0x01    /* Trigger rescheduling */
#define IPI_CALL_FUNC       0x02    /* Call function on CPU */
#define IPI_TLB_FLUSH       0x04    /* Flush TLB */
#define IPI_STOP            0x08    /* Stop CPU */
#define IPI_WAKEUP          0x10    /* Wake CPU from idle */
#define IPI_TIMER           0x20    /* Timer interrupt */

/* CPU hotplug states */
typedef enum {
    CPUHP_OFFLINE = 0,
    CPUHP_STARTING,
    CPUHP_AP_ONLINE_DYN,
    CPUHP_ONLINE,
    CPUHP_TEARDOWN_CPU
} cpuhp_state_t;

/* Function call data for cross-CPU function calls */
typedef struct smp_call_func_data {
    void (*func)(void *data);
    void *data;
    bool wait;
    volatile int done;
} smp_call_func_data_t;

/* CPU masks for affinity and operations */
typedef struct {
    unsigned long bits[MAX_CPUS / (sizeof(unsigned long) * 8) + 1];
} cpu_mask_t;

/* Global SMP state */
extern cpu_info_t cpu_data[MAX_CPUS];
extern uint32_t nr_cpus_online;
extern uint32_t nr_cpus_possible;
extern cpu_mask_t cpu_online_mask;
extern cpu_mask_t cpu_possible_mask;

/* SMP initialization functions */
int smp_init(void);
void smp_init_secondary(void);
int smp_boot_cpu(uint32_t cpu_id);
void smp_shutdown_cpu(uint32_t cpu_id);

/* CPU management */
uint32_t smp_processor_id(void);
cpu_info_t* smp_cpu_data(uint32_t cpu_id);
bool smp_cpu_online(uint32_t cpu_id);
void smp_set_cpu_state(uint32_t cpu_id, cpu_state_t state);

/* CPU topology */
void smp_detect_topology(void);
uint32_t smp_get_package_count(void);
uint32_t smp_get_core_count(uint32_t package_id);
bool smp_cores_share_cache(uint32_t cpu1, uint32_t cpu2, int cache_level);

/* IPI functions */
void smp_send_ipi(uint32_t cpu_id, uint32_t ipi_type);
void smp_send_ipi_mask(const cpu_mask_t *mask, uint32_t ipi_type);
void smp_send_ipi_all(uint32_t ipi_type);
void smp_send_ipi_all_but_self(uint32_t ipi_type);

/* Cross-CPU function calls */
void smp_call_function_single(uint32_t cpu_id, void (*func)(void *), void *data, bool wait);
void smp_call_function_many(const cpu_mask_t *mask, void (*func)(void *), void *data, bool wait);
void smp_call_function(void (*func)(void *), void *data, bool wait);

/* CPU mask operations */
void cpu_mask_clear(cpu_mask_t *mask);
void cpu_mask_set_cpu(uint32_t cpu, cpu_mask_t *mask);
void cpu_mask_clear_cpu(uint32_t cpu, cpu_mask_t *mask);
bool cpu_mask_test_cpu(uint32_t cpu, const cpu_mask_t *mask);
uint32_t cpu_mask_first(const cpu_mask_t *mask);
uint32_t cpu_mask_next(uint32_t cpu, const cpu_mask_t *mask);
uint32_t cpu_mask_weight(const cpu_mask_t *mask);
bool cpu_mask_empty(const cpu_mask_t *mask);

/* CPU hotplug support */
int cpu_up(uint32_t cpu_id);
int cpu_down(uint32_t cpu_id);
void cpu_hotplug_init(void);

/* Per-CPU variables support */
#define DEFINE_PER_CPU(type, name) \
    __attribute__((section(".percpu"))) type name[MAX_CPUS]

#define per_cpu(var, cpu) (var[cpu])
#define this_cpu(var) per_cpu(var, smp_processor_id())

/* Memory barriers for SMP */
#define smp_mb()    __asm__ __volatile__("mfence" ::: "memory")
#define smp_rmb()   __asm__ __volatile__("lfence" ::: "memory")
#define smp_wmb()   __asm__ __volatile__("sfence" ::: "memory")

/* Atomic operations for SMP coordination */
static inline void atomic_inc(volatile int *v) {
    __asm__ __volatile__("lock incl %0" : "+m" (*v));
}

static inline void atomic_dec(volatile int *v) {
    __asm__ __volatile__("lock decl %0" : "+m" (*v));
}

static inline int atomic_inc_return(volatile int *v) {
    int result;
    __asm__ __volatile__("lock incl %0; movl %0, %1" 
                        : "+m" (*v), "=r" (result));
    return result;
}

static inline bool atomic_dec_and_test(volatile int *v) {
    unsigned char c;
    __asm__ __volatile__("lock decl %0; sete %1"
                        : "+m" (*v), "=qm" (c)
                        :
                        : "memory");
    return c != 0;
}

/* Spinlock implementation for SMP */
typedef struct {
    volatile unsigned int slock;
    uint32_t owner_cpu;
    void *owner_pc;
} spinlock_t;

#define SPINLOCK_INIT { 0, 0, NULL }

void spin_lock_init(spinlock_t *lock);
void spin_lock(spinlock_t *lock);
void spin_unlock(spinlock_t *lock);
bool spin_trylock(spinlock_t *lock);
void spin_lock_irqsave(spinlock_t *lock, unsigned long *flags);
void spin_unlock_irqrestore(spinlock_t *lock, unsigned long flags);

/* Read-write locks */
typedef struct {
    volatile int lock;
    volatile int readers;
} rwlock_t;

#define RWLOCK_INIT { 0, 0 }

void rwlock_init(rwlock_t *lock);
void read_lock(rwlock_t *lock);
void read_unlock(rwlock_t *lock);
void write_lock(rwlock_t *lock);
void write_unlock(rwlock_t *lock);

/* CPU frequency scaling */
void smp_init_frequency_scaling(void);
void smp_set_cpu_frequency(uint32_t cpu_id, uint32_t freq_khz);
uint32_t smp_get_cpu_frequency(uint32_t cpu_id);

/* Power management */
void smp_enter_idle(void);
void smp_exit_idle(void);
void smp_cpu_relax(void);

/* NUMA support */
#define MAX_NUMA_NODES 64

typedef struct numa_node {
    uint32_t node_id;
    cpu_mask_t cpu_mask;        /* CPUs in this node */
    uint64_t memory_size;       /* Total memory in node */
    uint64_t free_memory;       /* Free memory in node */
    uint32_t distance[MAX_NUMA_NODES];  /* Distance to other nodes */
} numa_node_t;

extern numa_node_t numa_nodes[MAX_NUMA_NODES];
extern uint32_t nr_numa_nodes;

uint32_t numa_cpu_node(uint32_t cpu_id);
uint32_t numa_mem_node(void *addr);
void *numa_alloc_on_node(size_t size, uint32_t node_id);
void numa_init(void);

/* SMP debugging and monitoring */
void smp_dump_cpu_info(uint32_t cpu_id);
void smp_dump_all_cpus(void);
void smp_print_topology(void);
void smp_get_statistics(uint32_t cpu_id, void *stats);

/* CPU isolation and RT support */
void smp_isolate_cpu(uint32_t cpu_id);
void smp_unisolate_cpu(uint32_t cpu_id);
bool smp_cpu_isolated(uint32_t cpu_id);

/* CPU performance monitoring */
typedef struct cpu_perf_counters {
    uint64_t cycles;
    uint64_t instructions;
    uint64_t cache_misses;
    uint64_t branch_misses;
    uint64_t page_faults;
} cpu_perf_counters_t;

void smp_start_perf_monitoring(uint32_t cpu_id);
void smp_stop_perf_monitoring(uint32_t cpu_id);
void smp_get_perf_counters(uint32_t cpu_id, cpu_perf_counters_t *counters);

/* Macros for common operations */
#define for_each_possible_cpu(cpu) \
    for ((cpu) = cpu_mask_first(&cpu_possible_mask); \
         (cpu) < nr_cpus_possible; \
         (cpu) = cpu_mask_next((cpu), &cpu_possible_mask))

#define for_each_online_cpu(cpu) \
    for ((cpu) = cpu_mask_first(&cpu_online_mask); \
         (cpu) < nr_cpus_online; \
         (cpu) = cpu_mask_next((cpu), &cpu_online_mask))

#define for_each_cpu_in_mask(cpu, mask) \
    for ((cpu) = cpu_mask_first(mask); \
         (cpu) < MAX_CPUS; \
         (cpu) = cpu_mask_next((cpu), mask))

/* CPU capability flags */
#define CPU_CAP_FPU         0x00000001
#define CPU_CAP_VME         0x00000002
#define CPU_CAP_DE          0x00000004
#define CPU_CAP_PSE         0x00000008
#define CPU_CAP_TSC         0x00000010
#define CPU_CAP_MSR         0x00000020
#define CPU_CAP_PAE         0x00000040
#define CPU_CAP_MCE         0x00000080
#define CPU_CAP_CX8         0x00000100
#define CPU_CAP_APIC        0x00000200
#define CPU_CAP_SEP         0x00000800
#define CPU_CAP_MTRR        0x00001000
#define CPU_CAP_PGE         0x00002000
#define CPU_CAP_MCA         0x00004000
#define CPU_CAP_CMOV        0x00008000
#define CPU_CAP_FGPAT       0x00010000
#define CPU_CAP_PSE36       0x00020000
#define CPU_CAP_MMX         0x00800000
#define CPU_CAP_FXSR        0x01000000
#define CPU_CAP_XMM         0x02000000
#define CPU_CAP_XMM2        0x04000000

extern uint32_t cpu_capabilities[MAX_CPUS];

bool cpu_has_capability(uint32_t cpu_id, uint32_t capability);
void cpu_detect_capabilities(uint32_t cpu_id);