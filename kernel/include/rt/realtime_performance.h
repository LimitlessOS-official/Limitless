/*
 * LimitlessOS Real-time & Performance Core
 * Real-time scheduling, performance profiling, and system optimization
 */

#ifndef REALTIME_PERFORMANCE_H
#define REALTIME_PERFORMANCE_H

#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <pthread.h>

/* Real-time scheduling policies */
typedef enum {
    RT_POLICY_NORMAL,           /* Standard CFS */
    RT_POLICY_FIFO,            /* Real-time FIFO */
    RT_POLICY_RR,              /* Real-time Round Robin */
    RT_POLICY_DEADLINE,        /* Deadline scheduling */
    RT_POLICY_SPORADIC,        /* Sporadic task model */
    RT_POLICY_BATCH,           /* Batch processing */
    RT_POLICY_IDLE,            /* Idle priority */
    RT_POLICY_INTERACTIVE,     /* Interactive tasks */
    RT_POLICY_GAME_MODE,       /* Gaming optimization */
    RT_POLICY_LOW_LATENCY      /* Ultra-low latency */
} rt_policy_t;

/* Priority levels */
#define RT_PRIORITY_MIN     1
#define RT_PRIORITY_MAX     99
#define RT_PRIORITY_DEFAULT 50

/* Real-time task parameters */
typedef struct {
    uint32_t task_id;
    char name[64];
    rt_policy_t policy;
    uint32_t priority;
    
    /* Deadline scheduling parameters */
    uint64_t period;            /* Nanoseconds */
    uint64_t deadline;          /* Nanoseconds */
    uint64_t runtime;           /* Nanoseconds */
    
    /* CPU affinity */
    uint32_t cpu_affinity_mask;
    bool cpu_isolation;         /* Isolate on dedicated CPU */
    
    /* Memory constraints */
    bool memory_lock;           /* Lock pages in memory */
    uint64_t memory_limit;      /* Memory limit in bytes */
    
    /* I/O priority */
    enum {
        IO_PRIORITY_RT,         /* Real-time I/O */
        IO_PRIORITY_HIGH,       /* High priority */
        IO_PRIORITY_NORMAL,     /* Normal priority */
        IO_PRIORITY_LOW,        /* Low priority */
        IO_PRIORITY_IDLE        /* Idle I/O */
    } io_priority;
    
    /* Performance monitoring */
    struct {
        uint64_t exec_time;     /* Total execution time */
        uint64_t wait_time;     /* Total wait time */
        uint64_t deadline_misses;
        uint64_t context_switches;
        uint64_t page_faults;
        uint64_t cache_misses;
        double cpu_utilization;
        double memory_utilization;
    } stats;
} rt_task_t;

/* Performance counter types */
typedef enum {
    PERF_COUNTER_CPU_CYCLES,
    PERF_COUNTER_INSTRUCTIONS,
    PERF_COUNTER_CACHE_L1_MISSES,
    PERF_COUNTER_CACHE_L2_MISSES,
    PERF_COUNTER_CACHE_L3_MISSES,
    PERF_COUNTER_BRANCH_MISSES,
    PERF_COUNTER_TLB_MISSES,
    PERF_COUNTER_MEMORY_LOADS,
    PERF_COUNTER_MEMORY_STORES,
    PERF_COUNTER_CONTEXT_SWITCHES,
    PERF_COUNTER_PAGE_FAULTS,
    PERF_COUNTER_SYSCALLS,
    PERF_COUNTER_INTERRUPTS,
    PERF_COUNTER_NETWORK_PACKETS,
    PERF_COUNTER_DISK_IO,
    PERF_COUNTER_GPU_UTILIZATION,
    PERF_COUNTER_POWER_USAGE,
    PERF_COUNTER_THERMAL_EVENTS,
    PERF_COUNTER_MAX
} perf_counter_type_t;

/* Performance sample */
typedef struct {
    uint64_t timestamp;         /* Nanoseconds since boot */
    perf_counter_type_t type;
    uint64_t value;
    uint32_t cpu_id;
    uint32_t process_id;
    uint32_t thread_id;
    char process_name[32];
} perf_sample_t;

/* Performance profiler */
typedef struct {
    bool enabled;
    bool system_wide;           /* Profile entire system */
    uint32_t sample_rate;       /* Samples per second */
    uint64_t buffer_size;       /* Sample buffer size */
    
    /* Counter configuration */
    bool counters[PERF_COUNTER_MAX];
    uint32_t counter_count;
    
    /* Sampling */
    perf_sample_t *samples;
    uint64_t sample_count;
    uint64_t sample_index;
    pthread_mutex_t sample_mutex;
    
    /* Call stack profiling */
    bool stack_profiling;
    uint32_t stack_depth;
    
    /* Filtering */
    char process_filter[64];    /* Profile specific process */
    uint32_t cpu_filter;        /* Profile specific CPU */
    
    /* Output */
    char output_file[256];
    bool real_time_output;
    
    /* Analysis */
    struct {
        double cpu_usage[64];           /* Per-CPU usage */
        double memory_usage;            /* System memory usage */
        double network_usage;           /* Network utilization */
        double disk_usage;              /* Disk utilization */
        uint64_t hotspots[1024];        /* Function hotspots */
        uint32_t hotspot_count;
    } analysis;
} performance_profiler_t;

/* System optimization engine */
typedef struct {
    bool enabled;
    bool auto_optimization;     /* Automatic optimization */
    
    /* Optimization targets */
    enum {
        OPTIMIZE_LATENCY,       /* Minimize latency */
        OPTIMIZE_THROUGHPUT,    /* Maximize throughput */
        OPTIMIZE_POWER,         /* Power efficiency */
        OPTIMIZE_BALANCED,      /* Balanced performance */
        OPTIMIZE_GAMING,        /* Gaming performance */
        OPTIMIZE_SERVER,        /* Server workload */
        OPTIMIZE_DESKTOP        /* Desktop responsiveness */
    } target;
    
    /* CPU optimization */
    struct {
        bool frequency_scaling; /* Dynamic frequency scaling */
        bool governor_switching;/* CPU governor switching */
        bool core_parking;      /* CPU core parking */
        bool hyperthreading;    /* Hyperthreading control */
        bool cache_optimization;/* Cache line optimization */
        uint32_t idle_threshold;/* CPU idle threshold */
    } cpu;
    
    /* Memory optimization */
    struct {
        bool compaction;        /* Memory compaction */
        bool transparent_hugepages; /* THP management */
        bool numa_balancing;    /* NUMA page balancing */
        bool swap_optimization; /* Swap usage optimization */
        bool prefetching;       /* Memory prefetching */
        uint32_t swappiness;    /* Swap aggressiveness */
    } memory;
    
    /* I/O optimization */
    struct {
        bool io_scheduling;     /* I/O scheduler optimization */
        bool read_ahead;        /* Read-ahead optimization */
        bool write_back;        /* Write-back optimization */
        bool queue_depth;       /* I/O queue depth tuning */
        char scheduler[32];     /* I/O scheduler name */
    } io;
    
    /* Network optimization */
    struct {
        bool tcp_window_scaling;/* TCP window optimization */
        bool interrupt_coalescing; /* Network interrupt coalescing */
        bool rss_scaling;       /* Receive Side Scaling */
        bool gro_gso;          /* Generic Receive/Send Offload */
        uint32_t buffer_sizes;  /* Network buffer optimization */
    } network;
    
    /* Statistics */
    struct {
        uint64_t optimizations_applied;
        uint64_t performance_improvements;
        double avg_latency_reduction;
        double avg_throughput_increase;
        double power_savings;
    } stats;
} system_optimizer_t;

/* Low-latency I/O subsystem */
typedef struct {
    bool enabled;
    
    /* Bypass mechanisms */
    bool kernel_bypass;         /* Kernel bypass I/O */
    bool zero_copy;            /* Zero-copy networking */
    bool polling_mode;         /* Polling mode drivers */
    bool interrupt_mitigation; /* Interrupt mitigation */
    
    /* DPDK integration */
    struct {
        bool enabled;
        char eal_args[256];     /* DPDK EAL arguments */
        uint32_t hugepage_size; /* Hugepage size in MB */
        uint32_t memory_channels;
        uint32_t lcores;        /* Logical cores */
        char pci_whitelist[256]; /* PCI device whitelist */
    } dpdk;
    
    /* SPDK integration */
    struct {
        bool enabled;
        char config_file[256];  /* SPDK config file */
        bool nvme_over_fabrics; /* NVMe-oF support */
        bool vhost_user;        /* vhost-user support */
    } spdk;
    
    /* User-space networking */
    struct {
        bool enabled;
        char driver[32];        /* User-space driver */
        uint32_t ring_size;     /* Ring buffer size */
        uint32_t batch_size;    /* Packet batch size */
        bool busy_polling;      /* Busy polling */
    } userspace_net;
    
    /* Statistics */
    struct {
        uint64_t total_operations;
        uint64_t bypassed_operations;
        double avg_latency_us;  /* Average latency in microseconds */
        double min_latency_us;  /* Minimum latency */
        double max_latency_us;  /* Maximum latency */
        uint64_t operations_per_second;
        uint64_t bytes_per_second;
    } stats;
} lowlatency_io_t;

/* CPU isolation and partitioning */
typedef struct {
    bool enabled;
    
    /* CPU sets */
    struct {
        uint32_t system_cpus;       /* CPUs for system tasks */
        uint32_t realtime_cpus;     /* CPUs for real-time tasks */
        uint32_t application_cpus;  /* CPUs for applications */
        uint32_t isolated_cpus;     /* Completely isolated CPUs */
    } cpu_sets;
    
    /* Interrupt affinity */
    bool interrupt_isolation;   /* Isolate interrupts */
    uint32_t interrupt_cpus;    /* CPUs for interrupt handling */
    
    /* Tick management */
    bool nohz_full;            /* Tickless operation on isolated CPUs */
    bool rcu_nocbs;            /* RCU callback offloading */
    
    /* Workqueue isolation */
    bool workqueue_isolation;   /* Isolate kernel workqueues */
    
    /* Statistics */
    struct {
        uint64_t isolated_tasks;
        uint64_t cpu_migrations;
        double isolation_efficiency;
    } stats;
} cpu_isolation_t;

/* Real-time system configuration */
typedef struct {
    bool initialized;
    
    /* Real-time tasks */
    rt_task_t tasks[1024];      /* Up to 1024 RT tasks */
    uint32_t task_count;
    pthread_mutex_t task_mutex;
    
    /* Performance profiling */
    performance_profiler_t profiler;
    
    /* System optimization */
    system_optimizer_t optimizer;
    
    /* Low-latency I/O */
    lowlatency_io_t lowlat_io;
    
    /* CPU isolation */
    cpu_isolation_t cpu_isolation;
    
    /* Global settings */
    struct {
        bool preempt_rt;        /* PREEMPT_RT kernel */
        uint32_t tick_rate;     /* Kernel tick rate (Hz) */
        bool high_res_timers;   /* High-resolution timers */
        bool dynamic_ticks;     /* Dynamic tick system */
        uint32_t max_latency_us; /* Maximum allowed latency */
    } config;
    
    /* Statistics */
    struct {
        uint64_t total_rt_tasks;
        uint64_t deadline_misses;
        uint64_t context_switches;
        double avg_latency_us;
        double max_latency_us;
        double cpu_utilization;
        double memory_utilization;
        uint64_t optimizations_applied;
    } stats;
} realtime_system_t;

/* Function prototypes */

/* Real-time system initialization */
int rt_system_init(void);
void rt_system_exit(void);
int rt_system_configure(void);

/* Real-time task management */
int rt_task_create(const rt_task_t *task);
int rt_task_destroy(uint32_t task_id);
int rt_task_set_policy(uint32_t task_id, rt_policy_t policy, uint32_t priority);
int rt_task_set_deadline(uint32_t task_id, uint64_t period, uint64_t deadline, uint64_t runtime);
int rt_task_set_affinity(uint32_t task_id, uint32_t cpu_mask);
int rt_task_get_stats(uint32_t task_id, rt_task_t *stats);

/* Performance profiling */
int perf_profiler_init(void);
int perf_profiler_start(void);
int perf_profiler_stop(void);
int perf_profiler_configure(const performance_profiler_t *config);
int perf_profiler_enable_counter(perf_counter_type_t counter);
int perf_profiler_disable_counter(perf_counter_type_t counter);
int perf_profiler_sample(perf_sample_t *sample);
int perf_profiler_get_results(performance_profiler_t *results);
int perf_profiler_export_data(const char *filename);

/* System optimization */
int system_optimizer_init(void);
int system_optimizer_enable(void);
int system_optimizer_disable(void);
int system_optimizer_set_target(int target);
int system_optimizer_run_analysis(void);
int system_optimizer_apply_optimizations(void);
int system_optimizer_get_stats(system_optimizer_t *stats);

/* Low-latency I/O */
int lowlat_io_init(void);
int lowlat_io_enable_dpdk(const char *eal_args);
int lowlat_io_enable_spdk(const char *config_file);
int lowlat_io_enable_userspace_net(const char *driver);
int lowlat_io_measure_latency(double *avg_latency, double *min_latency, double *max_latency);
int lowlat_io_get_stats(lowlatency_io_t *stats);

/* CPU isolation */
int cpu_isolation_init(void);
int cpu_isolation_enable(void);
int cpu_isolation_set_cpuset(uint32_t system_cpus, uint32_t realtime_cpus, 
                            uint32_t application_cpus, uint32_t isolated_cpus);
int cpu_isolation_isolate_interrupts(uint32_t interrupt_cpus);
int cpu_isolation_enable_nohz_full(uint32_t cpu_mask);
int cpu_isolation_get_stats(cpu_isolation_t *stats);

/* Latency measurement */
int rt_measure_latency(uint32_t iterations, double *avg_latency, 
                      double *min_latency, double *max_latency);
int rt_measure_jitter(uint32_t duration_ms, double *jitter);
int rt_measure_context_switch_latency(double *latency);

/* Power management for performance */
int rt_set_cpu_governor(const char *governor);
int rt_set_cpu_frequency(uint32_t cpu_id, uint32_t frequency_mhz);
int rt_disable_cpu_idle_states(uint32_t cpu_mask);
int rt_enable_turbo_boost(bool enable);

/* Memory optimization for real-time */
int rt_lock_memory(void *addr, size_t size);
int rt_unlock_memory(void *addr, size_t size);
int rt_prefault_memory(void *addr, size_t size);
int rt_disable_swap(void);
int rt_enable_memory_compaction(bool enable);

/* Interrupt management */
int rt_set_interrupt_affinity(uint32_t irq, uint32_t cpu_mask);
int rt_disable_interrupt_balancing(void);
int rt_enable_interrupt_coalescing(bool enable);
int rt_set_interrupt_moderation(uint32_t usecs);

/* Utility functions */
const char *rt_policy_name(rt_policy_t policy);
const char *perf_counter_name(perf_counter_type_t counter);
uint64_t rt_get_time_ns(void);
uint64_t rt_get_cpu_cycles(void);
int rt_thread_set_priority(pthread_t thread, rt_policy_t policy, uint32_t priority);
int rt_process_set_priority(pid_t pid, rt_policy_t policy, uint32_t priority);

/* Global real-time system */
extern realtime_system_t rt_system;

#endif /* REALTIME_PERFORMANCE_H */