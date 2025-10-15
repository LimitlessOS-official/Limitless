/**
 * Advanced Real-Time Scheduler for LimitlessOS
 * Enterprise-grade real-time scheduling with deadline guarantees
 */

#ifndef REALTIME_SCHEDULER_H
#define REALTIME_SCHEDULER_H

#include "kernel.h"
#include "scheduler.h"

/* Real-time scheduling classes */
typedef enum {
    RT_CLASS_DEADLINE = 0,  /* Earliest Deadline First (EDF) */
    RT_CLASS_FIXED,         /* Fixed Priority (Rate Monotonic) */
    RT_CLASS_SPORADIC,      /* Sporadic Task Model */
    RT_CLASS_PERIODIC,      /* Periodic Task Model */
    RT_CLASS_APERIODIC,     /* Aperiodic with background service */
    RT_CLASS_COUNT
} rt_class_t;

/* Real-time task states */
typedef enum {
    RT_STATE_INACTIVE = 0,
    RT_STATE_READY,
    RT_STATE_RUNNING,
    RT_STATE_BLOCKED,
    RT_STATE_SUSPENDED,
    RT_STATE_DEADLINE_MISSED
} rt_state_t;

/* CPU isolation levels */
typedef enum {
    CPU_ISOLATION_NONE = 0,     /* No isolation */
    CPU_ISOLATION_SOFT,         /* Soft isolation - prefer dedicated CPU */
    CPU_ISOLATION_HARD,         /* Hard isolation - dedicated CPU only */
    CPU_ISOLATION_NOHZ_FULL    /* No-HZ full - completely isolated CPU */
} cpu_isolation_level_t;

/* Real-time task parameters */
typedef struct {
    /* Timing parameters */
    uint64_t period_ns;         /* Task period in nanoseconds */
    uint64_t deadline_ns;       /* Relative deadline in nanoseconds */
    uint64_t runtime_ns;        /* Maximum execution time per period */
    uint64_t priority;          /* Static priority (for fixed priority scheduling) */
    
    /* Advanced parameters */
    uint64_t jitter_tolerance_ns;   /* Allowed timing jitter */
    uint64_t wcet_ns;              /* Worst-case execution time */
    uint64_t bcet_ns;              /* Best-case execution time */
    
    /* Resource requirements */
    uint32_t cpu_affinity_mask;     /* CPU affinity mask */
    cpu_isolation_level_t isolation_level;
    uint32_t memory_bandwidth_mbps; /* Required memory bandwidth */
    uint32_t cache_partition;       /* Cache partition requirement */
    
    /* Quality of Service */
    uint32_t importance_level;      /* 1-10, higher = more important */
    uint32_t criticality_level;     /* 1-5, higher = more critical */
    bool preemption_disabled;       /* Disable preemption during execution */
} rt_task_params_t;

/* Real-time statistics per task */
typedef struct {
    uint64_t activations;           /* Number of task activations */
    uint64_t completions;           /* Number of successful completions */
    uint64_t deadline_misses;       /* Number of deadline misses */
    uint64_t preemptions;           /* Number of preemptions */
    
    uint64_t total_execution_time_ns;
    uint64_t min_execution_time_ns;
    uint64_t max_execution_time_ns;
    uint64_t avg_execution_time_ns;
    
    uint64_t total_response_time_ns;
    uint64_t min_response_time_ns;
    uint64_t max_response_time_ns;
    uint64_t avg_response_time_ns;
    
    uint64_t context_switches;
    uint64_t cache_misses;
    uint64_t page_faults;
} rt_task_stats_t;

/* Real-time task control block */
typedef struct rt_task {
    uint32_t task_id;
    pid_t pid;                      /* Associated process ID */
    rt_class_t class;
    rt_state_t state;
    rt_task_params_t params;
    rt_task_stats_t stats;
    
    /* Timing tracking */
    uint64_t next_period_start_ns;
    uint64_t current_deadline_ns;
    uint64_t activation_time_ns;
    uint64_t start_time_ns;
    uint64_t remaining_runtime_ns;
    
    /* Scheduling data */
    uint32_t assigned_cpu;
    uint32_t priority_boost;        /* Temporary priority boost */
    bool deadline_enforced;         /* Enforce deadline with killing */
    
    /* Resource tracking */
    uint64_t memory_used_bytes;
    uint64_t cache_footprint_kb;
    
    /* List linkage */
    struct rt_task* next;
    struct rt_task* prev;
    
    spinlock_t lock;
} rt_task_t;

/* Per-CPU real-time scheduling data */
typedef struct {
    uint32_t cpu_id;
    rt_task_t* current_task;        /* Currently running RT task */
    rt_task_t* ready_queue[RT_CLASS_COUNT];  /* Per-class ready queues */
    
    /* CPU-specific parameters */
    cpu_isolation_level_t isolation_level;
    bool rt_enabled;                /* RT scheduling enabled on this CPU */
    uint64_t rt_bandwidth_ns;       /* Available RT bandwidth per period */
    uint64_t rt_period_ns;          /* RT scheduling period */
    uint64_t rt_used_bandwidth_ns;  /* Currently used RT bandwidth */
    
    /* Performance tracking */
    uint64_t context_switches;
    uint64_t deadline_misses;
    uint64_t preemptions;
    uint64_t idle_time_ns;
    uint64_t rt_time_ns;
    
    spinlock_t lock;
} rt_cpu_data_t;

/* Global real-time scheduler state */
typedef struct {
    bool initialized;
    bool enabled;
    uint32_t cpu_count;
    
    /* Task management */
    rt_task_t* task_list;           /* Global task list */
    uint32_t task_count;
    uint32_t next_task_id;
    
    /* Per-CPU data */
    rt_cpu_data_t cpu_data[MAX_CPUS];
    
    /* Global configuration */
    uint64_t global_period_ns;      /* Global scheduling period */
    uint32_t rt_bandwidth_percent;  /* Percentage of CPU for RT tasks */
    bool admission_control;         /* Enable admission control */
    bool priority_inheritance;      /* Enable priority inheritance */
    
    /* Deadline scheduling parameters */
    bool edf_enabled;               /* Earliest Deadline First */
    uint64_t edf_threshold_ns;      /* EDF activation threshold */
    
    /* Performance monitoring */
    uint64_t scheduler_invocations;
    uint64_t scheduler_time_ns;
    uint64_t global_deadline_misses;
    uint64_t task_migrations;
    
    spinlock_t global_lock;
} realtime_scheduler_t;

/* Admission control result */
typedef enum {
    ADMISSION_ACCEPTED = 0,
    ADMISSION_REJECTED_CPU,         /* CPU utilization too high */
    ADMISSION_REJECTED_MEMORY,      /* Memory bandwidth exceeded */
    ADMISSION_REJECTED_DEADLINE,    /* Deadline not schedulable */
    ADMISSION_REJECTED_ISOLATION,   /* CPU isolation conflict */
    ADMISSION_REJECTED_RESOURCES    /* Other resource conflicts */
} admission_result_t;

/* Schedulability test results */
typedef struct {
    bool schedulable;
    uint32_t cpu_utilization_percent;
    uint64_t response_time_bound_ns;
    uint32_t priority_assignment[MAX_RT_TASKS];
    admission_result_t rejection_reason;
} schedulability_analysis_t;

/* Priority inheritance statistics */
typedef struct {
    uint64_t pi_activations;        /* Total PI activations */
    uint32_t active_pi_chains;      /* Currently active PI chains */
    uint32_t avg_chain_length;      /* Average chain length */
    uint32_t max_chain_length;      /* Maximum chain length observed */
} rt_pi_stats_t;

/* Real-time scheduler statistics */
typedef struct {
    bool enabled;
    uint32_t active_rt_tasks;
    uint32_t total_rt_tasks;
    
    /* Global performance */
    uint64_t total_activations;
    uint64_t total_completions;
    uint64_t total_deadline_misses;
    uint32_t deadline_miss_rate_ppm;    /* Parts per million */
    
    /* Timing statistics */
    uint64_t min_response_time_ns;
    uint64_t max_response_time_ns;
    uint64_t avg_response_time_ns;
    
    uint64_t min_execution_time_ns;
    uint64_t max_execution_time_ns;
    uint64_t avg_execution_time_ns;
    
    /* Resource utilization */
    uint32_t cpu_utilization_percent;
    uint32_t memory_bandwidth_utilization_percent;
    
    /* Per-CPU statistics */
    struct {
        uint32_t active_tasks;
        uint32_t rt_utilization_percent;
        cpu_isolation_level_t isolation_level;
        uint64_t context_switches;
        uint64_t deadline_misses;
    } per_cpu[MAX_CPUS];
    
    /* Scheduler overhead */
    uint64_t scheduler_invocations;
    uint64_t avg_scheduler_time_ns;
    uint64_t max_scheduler_time_ns;
} rt_scheduler_stats_t;

/* Function prototypes */

/* Initialization and configuration */
status_t realtime_scheduler_init(void);
status_t rt_scheduler_enable(bool enable);
status_t rt_scheduler_configure(uint32_t rt_bandwidth_percent, uint64_t global_period_ns);

/* CPU isolation management */
status_t rt_cpu_set_isolation(uint32_t cpu_id, cpu_isolation_level_t level);
status_t rt_cpu_enable_nohz_full(uint32_t cpu_id);
status_t rt_cpu_set_rt_bandwidth(uint32_t cpu_id, uint64_t bandwidth_ns, uint64_t period_ns);

/* Task management */
status_t rt_task_create(pid_t pid, rt_class_t class, const rt_task_params_t* params, uint32_t* task_id);
status_t rt_task_destroy(uint32_t task_id);
status_t rt_task_activate(uint32_t task_id);
status_t rt_task_suspend(uint32_t task_id);
status_t rt_task_set_params(uint32_t task_id, const rt_task_params_t* params);
status_t rt_task_get_stats(uint32_t task_id, rt_task_stats_t* stats);

/* Admission control and schedulability analysis */
admission_result_t rt_admission_control_test(const rt_task_params_t* params, uint32_t cpu_id);
status_t rt_schedulability_analysis(schedulability_analysis_t* analysis);
status_t rt_rate_monotonic_assignment(void);
status_t rt_deadline_monotonic_assignment(void);

/* Scheduling operations */
status_t rt_schedule(uint32_t cpu_id);
status_t rt_task_yield(uint32_t task_id);
status_t rt_task_sleep_until(uint32_t task_id, uint64_t wakeup_time_ns);
status_t rt_priority_inheritance_enable(uint32_t task_id, bool enable);

/* Performance monitoring */
status_t rt_get_scheduler_stats(rt_scheduler_stats_t* stats);
status_t rt_reset_statistics(void);
status_t rt_dump_task_stats(uint32_t task_id);
status_t rt_dump_cpu_stats(uint32_t cpu_id);
status_t rt_dump_all_stats(void);

/* Advanced features */
status_t rt_set_cache_partition(uint32_t task_id, uint32_t partition);
status_t rt_set_memory_bandwidth_reservation(uint32_t task_id, uint32_t bandwidth_mbps);
status_t rt_enable_deadline_enforcement(uint32_t task_id, bool enforce);
status_t rt_set_criticality_level(uint32_t task_id, uint32_t level);

/* System integration */
status_t rt_scheduler_tick(uint32_t cpu_id);
status_t rt_handle_deadline_miss(rt_task_t* task);
status_t rt_migrate_task(rt_task_t* task, uint32_t target_cpu);
status_t rt_balance_load(void);

/* Configuration constants */
#define MAX_RT_TASKS                    1024
#define DEFAULT_RT_BANDWIDTH_PERCENT    50
#define DEFAULT_RT_PERIOD_NS            (10 * 1000 * 1000)  /* 10ms */
#define MIN_TASK_PERIOD_NS              (100 * 1000)        /* 100μs */
#define MAX_TASK_PERIOD_NS              (1000 * 1000 * 1000) /* 1s */
#define RT_SCHEDULER_PRECISION_NS       1000                /* 1μs precision */

/* Priority levels */
#define RT_PRIORITY_HIGHEST             1
#define RT_PRIORITY_HIGH                50
#define RT_PRIORITY_NORMAL              100
#define RT_PRIORITY_LOW                 150
#define RT_PRIORITY_LOWEST              200

/* Criticality levels */
#define RT_CRITICALITY_SAFETY_CRITICAL  5
#define RT_CRITICALITY_MISSION_CRITICAL 4
#define RT_CRITICALITY_BUSINESS_CRITICAL 3
#define RT_CRITICALITY_IMPORTANT        2
#define RT_CRITICALITY_NORMAL           1

#endif /* REALTIME_SCHEDULER_H */