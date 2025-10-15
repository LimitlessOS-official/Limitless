#ifndef LIMITLESS_ENTERPRISE_METRICS_H
#define LIMITLESS_ENTERPRISE_METRICS_H

#include "types.h"

/* Maximum metric categories and entries */
#define METRICS_MAX_CATEGORIES 64
#define METRICS_MAX_ENTRIES_PER_CATEGORY 128
#define METRICS_CATEGORY_NAME_LEN 64
#define METRICS_ENTRY_NAME_LEN 128

/* Metric types */
typedef enum {
    METRIC_TYPE_COUNTER = 1,     /* Monotonic counter */
    METRIC_TYPE_GAUGE = 2,       /* Current value */
    METRIC_TYPE_HISTOGRAM = 3,   /* Value distribution */
    METRIC_TYPE_SUMMARY = 4,     /* Statistical summary */
    METRIC_TYPE_TIMER = 5        /* Time duration measurements */
} metric_type_t;

/* Metric aggregation methods */
typedef enum {
    METRIC_AGG_SUM = 1,
    METRIC_AGG_AVERAGE = 2,
    METRIC_AGG_MIN = 3,
    METRIC_AGG_MAX = 4,
    METRIC_AGG_COUNT = 5,
    METRIC_AGG_RATE = 6          /* Per second rate */
} metric_aggregation_t;

/* Individual metric value */
typedef union {
    uint64_t counter_value;
    int64_t gauge_value;
    double float_value;
    struct {
        uint64_t count;
        uint64_t sum;
        uint64_t min;
        uint64_t max;
        uint64_t buckets[16];    /* Histogram buckets */
    } histogram;
    struct {
        uint64_t count;
        double sum;
        double sum_squares;
        double min;
        double max;
    } summary;
} metric_value_t;

/* Metric metadata */
typedef struct {
    char name[METRICS_ENTRY_NAME_LEN];
    char description[256];
    metric_type_t type;
    metric_aggregation_t aggregation;
    uint64_t created_time;
    uint64_t last_updated;
    uint32_t update_count;
    metric_value_t value;
    spinlock_t lock;            /* Per-metric synchronization */
    bool enabled;
} metric_entry_t;

/* Metric category */
typedef struct {
    char name[METRICS_CATEGORY_NAME_LEN];
    char description[256];
    metric_entry_t entries[METRICS_MAX_ENTRIES_PER_CATEGORY];
    uint32_t entry_count;
    bool enabled;
    spinlock_t lock;
} metric_category_t;

/* System-wide metrics */
typedef struct {
    /* Performance metrics */
    uint64_t cpu_cycles;
    uint64_t context_switches;
    uint64_t interrupts;
    uint64_t page_faults;
    uint64_t system_calls;
    
    /* Memory metrics */
    uint64_t memory_allocated;
    uint64_t memory_freed;
    uint64_t memory_peak_usage;
    uint64_t memory_current_usage;
    uint32_t active_processes;
    
    /* I/O metrics */
    uint64_t disk_reads;
    uint64_t disk_writes;
    uint64_t disk_bytes_read;
    uint64_t disk_bytes_written;
    uint64_t network_packets_sent;
    uint64_t network_packets_received;
    uint64_t network_bytes_sent;
    uint64_t network_bytes_received;
    
    /* Security metrics */
    uint64_t authentication_attempts;
    uint64_t authentication_failures;
    uint64_t authorization_checks;
    uint64_t authorization_denials;
    uint64_t security_violations;
    uint64_t crypto_operations;
    
    /* Enterprise metrics */
    uint64_t backup_operations;
    uint64_t backup_failures;
    uint64_t audit_events;
    uint64_t compliance_checks;
    uint64_t policy_violations;
    uint64_t vpn_connections;
    uint64_t firewall_blocked_packets;
    
    /* Error metrics */
    uint64_t kernel_errors;
    uint64_t driver_errors;
    uint64_t filesystem_errors;
    uint64_t network_errors;
    
    uint64_t last_update_time;
} system_metrics_t;

/* Metrics collection configuration */
typedef struct {
    bool enabled;
    bool collect_performance_metrics;
    bool collect_security_metrics;
    bool collect_enterprise_metrics;
    bool collect_error_metrics;
    uint32_t collection_interval_ms;
    uint32_t retention_days;
    uint32_t max_entries_per_category;
    bool export_to_file;
    char export_path[256];
    bool compress_exports;
} metrics_config_t;

/* Metrics query filter */
typedef struct {
    char category_pattern[METRICS_CATEGORY_NAME_LEN];
    char name_pattern[METRICS_ENTRY_NAME_LEN];
    metric_type_t type_filter;
    uint64_t start_time;
    uint64_t end_time;
    uint32_t max_results;
    bool include_disabled;
} metrics_query_t;

/* Metrics export format */
typedef enum {
    METRICS_FORMAT_JSON = 1,
    METRICS_FORMAT_CSV = 2,
    METRICS_FORMAT_PROMETHEUS = 3,
    METRICS_FORMAT_INFLUXDB = 4,
    METRICS_FORMAT_BINARY = 5
} metrics_export_format_t;

/* Aggregated metric result */
typedef struct {
    char category_name[METRICS_CATEGORY_NAME_LEN];
    char metric_name[METRICS_ENTRY_NAME_LEN];
    metric_type_t type;
    metric_value_t aggregated_value;
    uint64_t sample_count;
    uint64_t first_timestamp;
    uint64_t last_timestamp;
} aggregated_metric_t;

/* Performance threshold alert */
typedef struct {
    char metric_name[METRICS_ENTRY_NAME_LEN];
    metric_type_t type;
    double threshold_value;
    bool threshold_exceeded;
    uint64_t first_exceeded_time;
    uint64_t last_checked_time;
    uint32_t exceeded_count;
    char alert_message[256];
} performance_alert_t;

/* Function declarations */
status_t enterprise_metrics_init(void);
status_t enterprise_metrics_shutdown(void);

/* Configuration */
status_t metrics_set_config(const metrics_config_t* config);
status_t metrics_get_config(metrics_config_t* out_config);
status_t metrics_enable_category(const char* category_name, bool enable);

/* Category management */
status_t metrics_create_category(const char* name, const char* description);
status_t metrics_delete_category(const char* name);
status_t metrics_list_categories(char names[][METRICS_CATEGORY_NAME_LEN], uint32_t* in_out_count);

/* Metric management */
status_t metrics_create_counter(const char* category, const char* name, const char* description);
status_t metrics_create_gauge(const char* category, const char* name, const char* description);
status_t metrics_create_histogram(const char* category, const char* name, const char* description);
status_t metrics_create_timer(const char* category, const char* name, const char* description);

/* Metric updates */
status_t metrics_increment_counter(const char* category, const char* name, uint64_t value);
status_t metrics_set_gauge(const char* category, const char* name, int64_t value);
status_t metrics_set_gauge_float(const char* category, const char* name, double value);
status_t metrics_record_histogram_value(const char* category, const char* name, uint64_t value);
status_t metrics_record_timer_duration(const char* category, const char* name, uint64_t duration_ns);

/* System metrics collection */
status_t metrics_update_system_metrics(void);
status_t metrics_get_system_metrics(system_metrics_t* out_metrics);
status_t metrics_reset_system_metrics(void);

/* Metric queries */
status_t metrics_query(const metrics_query_t* query, aggregated_metric_t* out_results, uint32_t* in_out_count);
status_t metrics_get_metric_value(const char* category, const char* name, metric_value_t* out_value);
status_t metrics_get_category_summary(const char* category, uint32_t* out_total_metrics, uint64_t* out_last_update);

/* Aggregation and statistics */
status_t metrics_aggregate_over_time(const char* category, const char* name,
                                    uint64_t start_time, uint64_t end_time,
                                    metric_aggregation_t method, double* out_result);
status_t metrics_calculate_rate(const char* category, const char* name,
                               uint64_t time_window_ms, double* out_rate);
status_t metrics_get_percentile(const char* category, const char* name,
                               double percentile, double* out_value);

/* Export and reporting */
status_t metrics_export_to_file(const char* file_path, metrics_export_format_t format);
status_t metrics_export_category(const char* category, const char* file_path, metrics_export_format_t format);
status_t metrics_generate_report(char* out_buffer, size_t buffer_size);
status_t metrics_get_top_metrics(const char* category, metric_aggregation_t sort_by,
                                uint32_t count, aggregated_metric_t* out_results);

/* Performance monitoring */
status_t metrics_set_performance_threshold(const char* category, const char* name,
                                          double threshold, const char* alert_message);
status_t metrics_check_performance_alerts(performance_alert_t* out_alerts, uint32_t* in_out_count);
status_t metrics_clear_performance_alert(const char* metric_name);

/* Real-time monitoring */
status_t metrics_start_real_time_collection(uint32_t interval_ms);
status_t metrics_stop_real_time_collection(void);
status_t metrics_register_update_callback(void (*callback)(const char* category, const char* name, const metric_value_t* value));

/* Utility functions */
status_t metrics_reset_metric(const char* category, const char* name);
status_t metrics_reset_category(const char* category);
status_t metrics_get_memory_usage(uint64_t* out_bytes);
status_t metrics_validate_metric_name(const char* name, bool* out_valid);

/* Debugging and diagnostics */
status_t metrics_dump_all_metrics(void);
status_t metrics_verify_integrity(bool* out_valid);
status_t metrics_get_collection_stats(uint64_t* out_updates, uint64_t* out_errors, uint64_t* out_memory_used);

/* Built-in system metrics categories */
#define METRICS_CATEGORY_SYSTEM     "system"
#define METRICS_CATEGORY_MEMORY     "memory"
#define METRICS_CATEGORY_CPU        "cpu"
#define METRICS_CATEGORY_IO         "io"
#define METRICS_CATEGORY_NETWORK    "network"
#define METRICS_CATEGORY_SECURITY   "security"
#define METRICS_CATEGORY_ENTERPRISE "enterprise"
#define METRICS_CATEGORY_ERRORS     "errors"

/* Common metric names */
#define METRIC_CPU_UTILIZATION      "cpu_utilization_percent"
#define METRIC_MEMORY_USAGE         "memory_usage_bytes"
#define METRIC_DISK_IO_RATE         "disk_io_rate_bps"
#define METRIC_NETWORK_THROUGHPUT   "network_throughput_bps"
#define METRIC_RESPONSE_TIME        "response_time_ms"
#define METRIC_ERROR_RATE           "error_rate_per_second"

#endif /* LIMITLESS_ENTERPRISE_METRICS_H */