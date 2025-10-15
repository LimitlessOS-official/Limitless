/**
 * System Monitoring and Telemetry Framework for LimitlessOS
 * Enterprise-grade monitoring with performance metrics and health monitoring
 */

#ifndef SYSTEM_MONITORING_H
#define SYSTEM_MONITORING_H

#include "kernel.h"

/* Monitoring subsystem types */
typedef enum {
    MONITOR_SUBSYS_CPU = 0,
    MONITOR_SUBSYS_MEMORY,
    MONITOR_SUBSYS_STORAGE,
    MONITOR_SUBSYS_NETWORK,
    MONITOR_SUBSYS_GRAPHICS,
    MONITOR_SUBSYS_PROCESS,
    MONITOR_SUBSYS_SECURITY,
    MONITOR_SUBSYS_FILESYSTEM,
    MONITOR_SUBSYS_REALTIME,
    MONITOR_SUBSYS_POWER,
    MONITOR_SUBSYS_COUNT
} monitor_subsystem_t;

/* Metric types */
typedef enum {
    METRIC_TYPE_COUNTER = 0,        /* Monotonically increasing value */
    METRIC_TYPE_GAUGE,              /* Point-in-time value */
    METRIC_TYPE_HISTOGRAM,          /* Distribution of values */
    METRIC_TYPE_SUMMARY,            /* Statistical summary */
    METRIC_TYPE_RATE,               /* Rate of change */
    METRIC_TYPE_PERCENTAGE,         /* Percentage value (0-100) */
    METRIC_TYPE_COUNT
} metric_type_t;

/* Alert severity levels */
typedef enum {
    ALERT_SEVERITY_INFO = 0,
    ALERT_SEVERITY_WARNING,
    ALERT_SEVERITY_ERROR,
    ALERT_SEVERITY_CRITICAL,
    ALERT_SEVERITY_EMERGENCY
} alert_severity_t;

/* Monitoring intervals */
typedef enum {
    MONITOR_INTERVAL_1S = 1000000000ULL,        /* 1 second */
    MONITOR_INTERVAL_5S = 5000000000ULL,        /* 5 seconds */
    MONITOR_INTERVAL_15S = 15000000000ULL,      /* 15 seconds */
    MONITOR_INTERVAL_30S = 30000000000ULL,      /* 30 seconds */
    MONITOR_INTERVAL_1M = 60000000000ULL,       /* 1 minute */
    MONITOR_INTERVAL_5M = 300000000000ULL,      /* 5 minutes */
    MONITOR_INTERVAL_15M = 900000000000ULL      /* 15 minutes */
} monitor_interval_t;

/* Metric value union */
typedef union {
    uint64_t counter;
    int64_t gauge;
    double percentage;
    struct {
        uint64_t count;
        uint64_t sum;
        uint64_t min;
        uint64_t max;
    } summary;
    struct {
        uint64_t buckets[16];       /* Histogram buckets */
        uint64_t bucket_bounds[16]; /* Bucket upper bounds */
        uint32_t bucket_count;
    } histogram;
} metric_value_t;

/* Individual metric */
typedef struct system_metric {
    uint32_t metric_id;
    char name[64];
    char description[128];
    monitor_subsystem_t subsystem;
    metric_type_t type;
    
    metric_value_t value;
    metric_value_t previous_value;
    
    uint64_t last_update_ns;
    uint64_t update_count;
    uint64_t collection_interval_ns;
    
    /* Thresholds for alerting */
    double warning_threshold;
    double critical_threshold;
    bool threshold_enabled;
    
    /* Labels for categorization */
    char labels[8][32];             /* Key-value pairs */
    uint32_t label_count;
    
    struct system_metric* next;
    spinlock_t lock;
} system_metric_t;

/* System health status */
typedef enum {
    HEALTH_STATUS_HEALTHY = 0,
    HEALTH_STATUS_DEGRADED,
    HEALTH_STATUS_UNHEALTHY,
    HEALTH_STATUS_CRITICAL,
    HEALTH_STATUS_UNKNOWN
} health_status_t;

/* Health check */
typedef struct health_check {
    uint32_t check_id;
    char name[64];
    char description[128];
    monitor_subsystem_t subsystem;
    
    health_status_t status;
    uint64_t last_check_ns;
    uint64_t check_interval_ns;
    uint32_t consecutive_failures;
    uint32_t failure_threshold;
    
    /* Check function pointer */
    health_status_t (*check_function)(void* context);
    void* context;
    
    /* Status history */
    health_status_t history[16];
    uint32_t history_index;
    
    struct health_check* next;
    spinlock_t lock;
} health_check_t;

/* Alert */
typedef struct system_alert {
    uint32_t alert_id;
    alert_severity_t severity;
    monitor_subsystem_t subsystem;
    
    char title[128];
    char message[512];
    char source[64];
    
    uint64_t timestamp_ns;
    uint64_t first_occurrence_ns;
    uint32_t occurrence_count;
    
    bool acknowledged;
    uint64_t acknowledged_at_ns;
    char acknowledged_by[32];
    
    bool resolved;
    uint64_t resolved_at_ns;
    
    /* Related metric */
    uint32_t related_metric_id;
    double trigger_value;
    
    struct system_alert* next;
    spinlock_t lock;
} system_alert_t;

/* Performance counters */
typedef struct {
    /* CPU metrics */
    uint64_t cpu_cycles_total;
    uint64_t cpu_instructions_total;
    uint32_t cpu_utilization_percent;
    uint32_t cpu_temperature_celsius;
    uint64_t context_switches_total;
    uint64_t interrupts_total;
    
    /* Memory metrics */
    uint64_t memory_total_bytes;
    uint64_t memory_free_bytes;
    uint64_t memory_used_bytes;
    uint64_t memory_cached_bytes;
    uint64_t memory_buffers_bytes;
    uint32_t memory_utilization_percent;
    uint64_t page_faults_total;
    uint64_t page_faults_major;
    
    /* Storage metrics */
    uint64_t disk_reads_total;
    uint64_t disk_writes_total;
    uint64_t disk_read_bytes_total;
    uint64_t disk_write_bytes_total;
    uint64_t disk_read_time_ns_total;
    uint64_t disk_write_time_ns_total;
    uint32_t disk_utilization_percent;
    uint64_t disk_errors_total;
    
    /* Network metrics */
    uint64_t network_rx_packets_total;
    uint64_t network_tx_packets_total;
    uint64_t network_rx_bytes_total;
    uint64_t network_tx_bytes_total;
    uint64_t network_rx_errors_total;
    uint64_t network_tx_errors_total;
    uint64_t network_rx_dropped_total;
    uint64_t network_tx_dropped_total;
    uint32_t network_utilization_percent;
    
    /* Process metrics */
    uint32_t processes_total;
    uint32_t processes_running;
    uint32_t processes_sleeping;
    uint32_t processes_zombie;
    uint32_t threads_total;
    uint64_t fork_total;
    uint64_t exec_total;
    
    /* Security metrics */
    uint64_t security_violations_total;
    uint64_t auth_failures_total;
    uint64_t crypto_operations_total;
    uint64_t audit_events_total;
    
    /* Filesystem metrics */
    uint64_t fs_reads_total;
    uint64_t fs_writes_total;
    uint64_t fs_opens_total;
    uint64_t fs_closes_total;
    uint64_t fs_errors_total;
    
    /* Real-time metrics */
    uint64_t rt_activations_total;
    uint64_t rt_deadline_misses_total;
    uint64_t rt_context_switches_total;
    uint32_t rt_utilization_percent;
    
    /* System uptime and load */
    uint64_t uptime_seconds;
    double load_average_1min;
    double load_average_5min;
    double load_average_15min;
} performance_counters_t;

/* Telemetry export formats */
typedef enum {
    TELEMETRY_FORMAT_JSON = 0,
    TELEMETRY_FORMAT_PROMETHEUS,
    TELEMETRY_FORMAT_INFLUXDB,
    TELEMETRY_FORMAT_GRAPHITE,
    TELEMETRY_FORMAT_STATSD,
    TELEMETRY_FORMAT_CUSTOM
} telemetry_format_t;

/* Telemetry endpoint */
typedef struct {
    uint32_t endpoint_id;
    char name[64];
    char url[256];
    telemetry_format_t format;
    uint64_t export_interval_ns;
    uint64_t last_export_ns;
    
    bool enabled;
    uint64_t exports_total;
    uint64_t export_errors_total;
    uint64_t bytes_exported_total;
    
    spinlock_t lock;
} telemetry_endpoint_t;

/* Main monitoring system state */
typedef struct {
    bool initialized;
    bool enabled;
    uint64_t start_time_ns;
    
    /* Metrics management */
    system_metric_t* metric_list;
    uint32_t metric_count;
    uint32_t next_metric_id;
    
    /* Health monitoring */
    health_check_t* health_check_list;
    uint32_t health_check_count;
    uint32_t next_health_check_id;
    health_status_t overall_health;
    
    /* Alert management */
    system_alert_t* alert_list;
    uint32_t alert_count;
    uint32_t active_alert_count;
    uint32_t next_alert_id;
    
    /* Performance counters */
    performance_counters_t counters;
    performance_counters_t previous_counters;
    uint64_t counter_update_interval_ns;
    uint64_t last_counter_update_ns;
    
    /* Telemetry endpoints */
    telemetry_endpoint_t endpoints[16];
    uint32_t endpoint_count;
    
    /* Configuration */
    uint32_t max_metrics;
    uint32_t max_alerts;
    uint64_t metric_retention_ns;
    uint64_t alert_retention_ns;
    
    /* Statistics */
    uint64_t monitoring_cycles;
    uint64_t monitoring_time_ns;
    uint64_t data_points_collected;
    uint64_t alerts_generated;
    
    spinlock_t global_lock;
    spinlock_t metric_lock;
    spinlock_t health_lock;
    spinlock_t alert_lock;
} system_monitoring_t;

/* Monitoring configuration */
typedef struct {
    monitor_interval_t default_interval;
    uint32_t max_metrics_per_subsystem;
    uint32_t max_alerts_per_subsystem;
    bool auto_cleanup_enabled;
    uint64_t cleanup_interval_ns;
    
    /* Threshold defaults */
    double cpu_warning_threshold;
    double cpu_critical_threshold;
    double memory_warning_threshold;
    double memory_critical_threshold;
    double disk_warning_threshold;
    double disk_critical_threshold;
    
    /* Export settings */
    bool prometheus_enabled;
    uint16_t prometheus_port;
    bool json_export_enabled;
    char json_export_path[256];
} monitoring_config_t;

/* Statistics and reporting */
typedef struct {
    uint64_t metrics_collected;
    uint64_t health_checks_performed;
    uint64_t alerts_active;
    uint64_t alerts_resolved;
    
    uint32_t subsystem_health[MONITOR_SUBSYS_COUNT];
    uint32_t subsystem_metric_count[MONITOR_SUBSYS_COUNT];
    uint32_t subsystem_alert_count[MONITOR_SUBSYS_COUNT];
    
    double average_collection_time_ns;
    uint64_t max_collection_time_ns;
    uint64_t total_collection_time_ns;
    
    uint64_t memory_usage_bytes;
    uint32_t monitoring_overhead_percent;
} monitoring_stats_t;

/* Function prototypes */

/* Initialization and configuration */
status_t system_monitoring_init(void);
status_t monitoring_enable(bool enable);
status_t monitoring_configure(const monitoring_config_t* config);
status_t monitoring_set_intervals(monitor_subsystem_t subsystem, monitor_interval_t interval);

/* Metric management */
status_t monitoring_register_metric(const char* name, const char* description,
                                  monitor_subsystem_t subsystem, metric_type_t type,
                                  uint32_t* metric_id);
status_t monitoring_update_metric(uint32_t metric_id, const metric_value_t* value);
status_t monitoring_set_metric_threshold(uint32_t metric_id, double warning, double critical);
status_t monitoring_get_metric(uint32_t metric_id, system_metric_t* metric);
status_t monitoring_list_metrics(monitor_subsystem_t subsystem, uint32_t** metric_ids, uint32_t* count);

/* Health monitoring */
status_t monitoring_register_health_check(const char* name, const char* description,
                                        monitor_subsystem_t subsystem,
                                        health_status_t (*check_func)(void*),
                                        void* context, uint32_t* check_id);
status_t monitoring_run_health_checks(monitor_subsystem_t subsystem);
status_t monitoring_get_health_status(monitor_subsystem_t subsystem, health_status_t* status);
status_t monitoring_get_overall_health(health_status_t* status);

/* Alert management */
status_t monitoring_create_alert(alert_severity_t severity, monitor_subsystem_t subsystem,
                               const char* title, const char* message, const char* source,
                               uint32_t* alert_id);
status_t monitoring_acknowledge_alert(uint32_t alert_id, const char* acknowledged_by);
status_t monitoring_resolve_alert(uint32_t alert_id);
status_t monitoring_list_active_alerts(uint32_t** alert_ids, uint32_t* count);
status_t monitoring_get_alert(uint32_t alert_id, system_alert_t* alert);

/* Performance counter collection */
status_t monitoring_update_counters(void);
status_t monitoring_get_counters(performance_counters_t* counters);
status_t monitoring_reset_counters(void);

/* Telemetry and export */
status_t monitoring_add_telemetry_endpoint(const char* name, const char* url,
                                         telemetry_format_t format, 
                                         uint64_t export_interval_ns,
                                         uint32_t* endpoint_id);
status_t monitoring_export_telemetry(uint32_t endpoint_id);
status_t monitoring_export_all_telemetry(void);
status_t monitoring_generate_prometheus_metrics(char* buffer, size_t buffer_size);
status_t monitoring_generate_json_report(char* buffer, size_t buffer_size);

/* System integration */
status_t monitoring_collect_cpu_metrics(void);
status_t monitoring_collect_memory_metrics(void);
status_t monitoring_collect_storage_metrics(void);
status_t monitoring_collect_network_metrics(void);
status_t monitoring_collect_all_metrics(void);

/* Maintenance and cleanup */
status_t monitoring_cleanup_old_data(void);
status_t monitoring_optimize_storage(void);
status_t monitoring_run_maintenance(void);

/* Statistics and reporting */
status_t monitoring_get_stats(monitoring_stats_t* stats);
status_t monitoring_dump_all_stats(void);
status_t monitoring_generate_health_report(void);
status_t monitoring_generate_performance_report(void);

/* Utility functions */
const char* monitoring_subsystem_name(monitor_subsystem_t subsystem);
const char* monitoring_metric_type_name(metric_type_t type);
const char* monitoring_health_status_name(health_status_t status);
const char* monitoring_alert_severity_name(alert_severity_t severity);

/* Configuration constants */
#define MAX_METRICS                     4096
#define MAX_HEALTH_CHECKS              1024
#define MAX_ALERTS                     2048
#define MAX_TELEMETRY_ENDPOINTS        16
#define DEFAULT_METRIC_RETENTION_DAYS  30
#define DEFAULT_ALERT_RETENTION_DAYS   90
#define MONITORING_THREAD_STACK_SIZE   (64 * 1024)

/* Default thresholds */
#define DEFAULT_CPU_WARNING_THRESHOLD   80.0
#define DEFAULT_CPU_CRITICAL_THRESHOLD  95.0
#define DEFAULT_MEM_WARNING_THRESHOLD   80.0
#define DEFAULT_MEM_CRITICAL_THRESHOLD  95.0
#define DEFAULT_DISK_WARNING_THRESHOLD  85.0
#define DEFAULT_DISK_CRITICAL_THRESHOLD 95.0

/* Metric update macros */
#define METRIC_INCREMENT(id) monitoring_increment_counter(id, 1)
#define METRIC_ADD(id, value) monitoring_increment_counter(id, value)
#define METRIC_SET(id, value) monitoring_set_gauge(id, value)
#define METRIC_OBSERVE(id, value) monitoring_observe_value(id, value)

/* Health check macros */
#define HEALTH_CHECK_OK() HEALTH_STATUS_HEALTHY
#define HEALTH_CHECK_WARN() HEALTH_STATUS_DEGRADED
#define HEALTH_CHECK_ERROR() HEALTH_STATUS_UNHEALTHY
#define HEALTH_CHECK_CRITICAL() HEALTH_STATUS_CRITICAL

#endif /* SYSTEM_MONITORING_H */