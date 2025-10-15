/*
 * observability.h - LimitlessOS Observability Framework Header
 * 
 * Provides structured logging, tracing, metrics, and crash analysis.
 */

#ifndef LIMITLESSOS_OBSERVABILITY_H
#define LIMITLESSOS_OBSERVABILITY_H

#include <kernel/types.h>
#include <kernel/process.h>
#include <kernel/timer.h>

/* Log levels */
typedef enum {
    LOG_LEVEL_TRACE = 0,
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARN,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_FATAL
} log_level_t;

/* Trace event types */
typedef enum {
    TRACE_EVENT_FUNCTION_ENTER,
    TRACE_EVENT_FUNCTION_EXIT,
    TRACE_EVENT_SYSCALL_ENTER,
    TRACE_EVENT_SYSCALL_EXIT,
    TRACE_EVENT_INTERRUPT,
    TRACE_EVENT_CONTEXT_SWITCH,
    TRACE_EVENT_PAGE_FAULT,
    TRACE_EVENT_MEMORY_ALLOC,
    TRACE_EVENT_MEMORY_FREE,
    TRACE_EVENT_LOCK_ACQUIRE,
    TRACE_EVENT_LOCK_RELEASE,
    TRACE_EVENT_CUSTOM
} trace_event_type_t;

/* Metric types */
typedef enum {
    METRIC_TYPE_COUNTER,
    METRIC_TYPE_GAUGE,
    METRIC_TYPE_HISTOGRAM,
    METRIC_TYPE_SUMMARY
} metric_type_t;

/* CPU context for crash dumps */
typedef struct cpu_context {
    uint64_t rax, rbx, rcx, rdx;
    uint64_t rsi, rdi, rbp, rsp;
    uint64_t r8, r9, r10, r11;
    uint64_t r12, r13, r14, r15;
    uint64_t rip, rflags;
    uint16_t cs, ds, es, fs, gs, ss;
} cpu_context_t;

/* System statistics */
typedef struct system_stats {
    uint64_t uptime_seconds;
    uint32_t total_processes;
    uint32_t log_entries;
    uint32_t trace_events;
    uint32_t crash_dumps;
    uint64_t memory_total;
    uint64_t memory_used;
    uint64_t memory_free;
    uint32_t cpu_usage_percent;
} system_stats_t;

/* Core observability functions */
status_t observability_init(void);

/* Structured logging */
void obs_log(log_level_t level, const char* subsystem, const char* file, 
            uint32_t line, const char* format, ...);

/* Tracing */
void obs_trace(trace_event_type_t type, uint64_t data1, uint64_t data2, 
              uint64_t data3, const char* description);

/* Performance counters */
void obs_counter_inc(const char* name, uint64_t value);
void obs_counter_set(const char* name, uint64_t value);

/* Metrics */
void obs_metric_counter_inc(const char* name, const char* labels[], uint64_t value);
void obs_metric_gauge_set(const char* name, const char* labels[], int64_t value);
void obs_metric_histogram_observe(const char* name, const char* labels[], uint64_t value);

/* Crash dump handling */
void obs_create_crash_dump(process_t* process, uint32_t signal, void* crash_addr,
                          cpu_context_t* context);

/* Configuration */
void obs_enable_tracing(bool enabled);
void obs_set_trace_filter(uint32_t filter_mask);
void obs_set_log_level(log_level_t level);
void obs_enable_profiling(bool enabled);

/* Data export */
size_t obs_export_metrics_prometheus(char* buffer, size_t buffer_size);
void obs_get_system_stats(system_stats_t* stats);

/* Performance profiling */
void obs_profile_function_enter(const char* function_name, void* caller_addr);
void obs_profile_function_exit(const char* function_name, void* caller_addr);

/* Real-time monitoring */
void obs_start_realtime_monitoring(uint32_t sample_rate_hz);
void obs_sampling_callback(void* context);

/* Helper functions */
uint32_t generate_stack_trace(process_t* process, void** trace, uint32_t max_frames);
uint32_t get_process_memory_maps(process_t* process, void* maps, uint32_t max_maps);
void create_core_dump(process_t* process, void* buffer, size_t size);
uint32_t cpu_get_usage_percent(void);

/* Convenient logging macros */
#define OBS_TRACE(subsystem, ...) obs_log(LOG_LEVEL_TRACE, subsystem, __FILE__, __LINE__, __VA_ARGS__)
#define OBS_DEBUG(subsystem, ...) obs_log(LOG_LEVEL_DEBUG, subsystem, __FILE__, __LINE__, __VA_ARGS__)
#define OBS_INFO(subsystem, ...)  obs_log(LOG_LEVEL_INFO, subsystem, __FILE__, __LINE__, __VA_ARGS__)
#define OBS_WARN(subsystem, ...)  obs_log(LOG_LEVEL_WARN, subsystem, __FILE__, __LINE__, __VA_ARGS__)
#define OBS_ERROR(subsystem, ...) obs_log(LOG_LEVEL_ERROR, subsystem, __FILE__, __LINE__, __VA_ARGS__)
#define OBS_FATAL(subsystem, ...) obs_log(LOG_LEVEL_FATAL, subsystem, __FILE__, __LINE__, __VA_ARGS__)

/* Function instrumentation macros */
#define PROFILE_ENTER() obs_profile_function_enter(__func__, __builtin_return_address(0))
#define PROFILE_EXIT()  obs_profile_function_exit(__func__, __builtin_return_address(0))

/* Trace macros */
#define TRACE_FUNC_ENTER() obs_trace(TRACE_EVENT_FUNCTION_ENTER, 0, 0, 0, __func__)
#define TRACE_FUNC_EXIT()  obs_trace(TRACE_EVENT_FUNCTION_EXIT, 0, 0, 0, __func__)

/* Counter macros */
#define COUNT_INC(name) obs_counter_inc(name, 1)
#define COUNT_ADD(name, val) obs_counter_inc(name, val)

/* Metric macros */
#define METRIC_COUNTER(name, val) obs_metric_counter_inc(name, NULL, val)
#define METRIC_GAUGE(name, val) obs_metric_gauge_set(name, NULL, val)
#define METRIC_TIME(name, val) obs_metric_histogram_observe(name, NULL, val)

#endif /* LIMITLESSOS_OBSERVABILITY_H */