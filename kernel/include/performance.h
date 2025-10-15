/**
 * Process Performance Monitoring and Profiling System for LimitlessOS
 * Provides comprehensive performance metrics and profiling capabilities
 */

#pragma once
#include "kernel.h"
#include "process.h"

/* Performance event types */
typedef enum {
    PERF_EVENT_CPU_CYCLES = 0,
    PERF_EVENT_INSTRUCTIONS,
    PERF_EVENT_CACHE_MISSES,
    PERF_EVENT_BRANCH_MISSES,
    PERF_EVENT_PAGE_FAULTS,
    PERF_EVENT_CONTEXT_SWITCHES,
    PERF_EVENT_SYSCALLS,
    PERF_EVENT_INTERRUPTS,
    PERF_EVENT_IO_READS,
    PERF_EVENT_IO_WRITES,
    PERF_EVENT_NETWORK_RX,
    PERF_EVENT_NETWORK_TX,
    PERF_EVENT_MEMORY_ALLOC,
    PERF_EVENT_MEMORY_FREE,
    PERF_EVENT_MAX
} perf_event_type_t;

/* Performance counter */
typedef struct perf_counter {
    perf_event_type_t type;
    uint64_t count;
    uint64_t timestamp;
    bool enabled;
} perf_counter_t;

/* Performance sample */
typedef struct perf_sample {
    uint64_t timestamp;
    uint32_t cpu_id;
    pid_t pid;
    tid_t tid;
    
    uint64_t instruction_pointer;
    uint64_t stack_pointer;
    
    perf_event_type_t event_type;
    uint64_t event_count;
    
    /* Call stack sample (simplified) */
    uint64_t callstack[16];
    uint32_t callstack_depth;
} perf_sample_t;

/* Performance profile data */
typedef struct perf_profile {
    pid_t pid;
    uint64_t start_time;
    uint64_t end_time;
    
    perf_counter_t counters[PERF_EVENT_MAX];
    
    /* Sample buffer */
    perf_sample_t* samples;
    uint32_t sample_count;
    uint32_t max_samples;
    
    /* Hot spots analysis */
    struct {
        uint64_t address;
        uint64_t hit_count;
        float percentage;
    } hotspots[100];
    uint32_t hotspot_count;
    
    /* Function call statistics */
    struct {
        uint64_t function_address;
        char function_name[64];
        uint64_t call_count;
        uint64_t total_time;
        uint64_t min_time;
        uint64_t max_time;
        float avg_time;
    } function_stats[256];
    uint32_t function_count;
    
    bool active;
    spinlock_t lock;
} perf_profile_t;

/* System-wide performance metrics */
typedef struct system_performance {
    /* CPU metrics */
    uint64_t total_cpu_time;
    uint64_t idle_time;
    uint64_t kernel_time;
    uint64_t user_time;
    float cpu_usage_percent;
    
    /* Memory metrics */
    size_t total_memory;
    size_t used_memory;
    size_t free_memory;
    size_t cached_memory;
    size_t buffer_memory;
    float memory_usage_percent;
    
    /* I/O metrics */
    uint64_t total_io_reads;
    uint64_t total_io_writes;
    uint64_t io_bytes_read;
    uint64_t io_bytes_written;
    float io_usage_percent;
    
    /* Network metrics */
    uint64_t network_packets_rx;
    uint64_t network_packets_tx;
    uint64_t network_bytes_rx;
    uint64_t network_bytes_tx;
    
    /* Process metrics */
    uint32_t process_count;
    uint32_t thread_count;
    uint32_t zombie_count;
    
    /* System load */
    float load_average_1min;
    float load_average_5min;
    float load_average_15min;
    
    uint64_t last_update_time;
} system_performance_t;

/* Performance monitoring API */
status_t perf_init(void);
status_t perf_shutdown(void);

/* Process profiling */
status_t perf_start_profiling(process_t* proc, uint32_t sample_rate);
status_t perf_stop_profiling(process_t* proc);
status_t perf_get_profile_data(process_t* proc, perf_profile_t** profile);
status_t perf_clear_profile_data(process_t* proc);

/* Performance counters */
status_t perf_enable_counter(process_t* proc, perf_event_type_t event);
status_t perf_disable_counter(process_t* proc, perf_event_type_t event);
status_t perf_read_counter(process_t* proc, perf_event_type_t event, uint64_t* count);
status_t perf_reset_counter(process_t* proc, perf_event_type_t event);

/* System performance monitoring */
status_t perf_get_system_metrics(system_performance_t* metrics);
status_t perf_update_system_metrics(void);
status_t perf_get_load_average(float* load1, float* load5, float* load15);

/* Performance events */
status_t perf_record_event(process_t* proc, perf_event_type_t event, uint64_t count);
status_t perf_record_sample(process_t* proc, perf_event_type_t event, uint64_t ip, uint64_t sp);

/* Analysis and reporting */
status_t perf_analyze_hotspots(perf_profile_t* profile);
status_t perf_analyze_function_calls(perf_profile_t* profile);
status_t perf_generate_report(perf_profile_t* profile, char* buffer, size_t buffer_size);

/* Resource usage tracking */
status_t perf_track_memory_usage(process_t* proc, size_t allocation_size, bool is_allocation);
status_t perf_track_io_usage(process_t* proc, size_t bytes, bool is_read);
status_t perf_track_network_usage(process_t* proc, size_t bytes, bool is_rx);

/* Performance alerts and thresholds */
typedef void (*perf_alert_callback_t)(process_t* proc, perf_event_type_t event, uint64_t threshold, uint64_t current);

status_t perf_set_threshold(process_t* proc, perf_event_type_t event, uint64_t threshold, perf_alert_callback_t callback);
status_t perf_remove_threshold(process_t* proc, perf_event_type_t event);

/* Performance optimization suggestions */
typedef enum {
    PERF_SUGGESTION_CPU_BOUND = 0,
    PERF_SUGGESTION_MEMORY_BOUND,
    PERF_SUGGESTION_IO_BOUND,
    PERF_SUGGESTION_NETWORK_BOUND,
    PERF_SUGGESTION_LOCK_CONTENTION,
    PERF_SUGGESTION_CACHE_MISSES,
    PERF_SUGGESTION_EXCESSIVE_SYSCALLS,
    PERF_SUGGESTION_MEMORY_LEAKS
} perf_suggestion_type_t;

typedef struct perf_suggestion {
    perf_suggestion_type_t type;
    char description[256];
    float impact_score;  /* 0.0 to 1.0 */
    char recommendation[512];
} perf_suggestion_t;

status_t perf_analyze_performance(process_t* proc, perf_suggestion_t* suggestions, uint32_t* count);