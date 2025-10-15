/*
 * LimitlessOS Enterprise Monitoring & Analytics Header
 * Comprehensive system monitoring with AI-powered insights and predictive maintenance
 */

#ifndef ENTERPRISE_MONITORING_H
#define ENTERPRISE_MONITORING_H

#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <pthread.h>
#include <sys/types.h>

/* Maximum limits */
#define MAX_METRICS 1000
#define MAX_ALERTS 500
#define MAX_DASHBOARDS 50
#define MAX_QUERIES 100
#define MAX_PREDICTIONS 200
#define MAX_THRESHOLDS 1000
#define MAX_SERVICES 500
#define MAX_NODES 100

/* Metric types */
typedef enum {
    METRIC_COUNTER = 0,      /* Ever-increasing value */
    METRIC_GAUGE,            /* Point-in-time value */
    METRIC_HISTOGRAM,        /* Distribution of values */
    METRIC_SUMMARY,          /* Summary statistics */
    METRIC_TIMER,            /* Time-based measurements */
    METRIC_RATE,             /* Rate of change */
    METRIC_PERCENTAGE,       /* Percentage values */
    METRIC_BOOLEAN,          /* Binary state */
    METRIC_MAX
} metric_type_t;

/* Data source types */
typedef enum {
    SOURCE_SYSTEM = 0,       /* OS system metrics */
    SOURCE_HARDWARE,         /* Hardware sensors */
    SOURCE_APPLICATION,      /* Application metrics */
    SOURCE_NETWORK,          /* Network statistics */
    SOURCE_DATABASE,         /* Database metrics */
    SOURCE_CONTAINER,        /* Container metrics */
    SOURCE_SECURITY,         /* Security events */
    SOURCE_PERFORMANCE,      /* Performance counters */
    SOURCE_CUSTOM,           /* User-defined */
    SOURCE_MAX
} data_source_t;

/* Alert severity levels */
typedef enum {
    ALERT_INFO = 0,
    ALERT_WARNING,
    ALERT_CRITICAL,
    ALERT_EMERGENCY,
    ALERT_MAX
} alert_severity_t;

/* Aggregation methods */
typedef enum {
    AGG_NONE = 0,
    AGG_AVERAGE,
    AGG_SUM,
    AGG_MIN,
    AGG_MAX,
    AGG_COUNT,
    AGG_MEDIAN,
    AGG_PERCENTILE,
    AGG_RATE,
    AGG_MAX_METHOD
} aggregation_method_t;

/* Time intervals */
typedef enum {
    INTERVAL_1S = 1,
    INTERVAL_5S = 5,
    INTERVAL_10S = 10,
    INTERVAL_30S = 30,
    INTERVAL_1M = 60,
    INTERVAL_5M = 300,
    INTERVAL_15M = 900,
    INTERVAL_1H = 3600,
    INTERVAL_1D = 86400
} time_interval_t;

/* Prediction models */
typedef enum {
    MODEL_LINEAR_REGRESSION = 0,
    MODEL_EXPONENTIAL_SMOOTHING,
    MODEL_ARIMA,
    MODEL_LSTM_NEURAL_NETWORK,
    MODEL_RANDOM_FOREST,
    MODEL_PROPHET,
    MODEL_ENSEMBLE,
    MODEL_MAX
} prediction_model_t;

/* Metric data point */
typedef struct {
    time_t timestamp;
    double value;
    char tags[256];          /* Key-value pairs for filtering */
    uint32_t quality;        /* Data quality score 0-100 */
} metric_point_t;

/* Metric definition */
typedef struct {
    uint32_t metric_id;
    char name[128];
    char description[512];
    char unit[32];
    metric_type_t type;
    data_source_t source;
    aggregation_method_t aggregation;
    time_interval_t interval;
    
    /* Current value */
    double current_value;
    time_t last_updated;
    
    /* Historical data */
    metric_point_t *history;
    uint32_t history_count;
    uint32_t history_capacity;
    
    /* Statistics */
    double min_value;
    double max_value;
    double avg_value;
    double std_deviation;
    
    /* Thresholds */
    double warning_threshold;
    double critical_threshold;
    bool threshold_enabled;
    
    /* Collection settings */
    bool enabled;
    uint32_t collection_interval; /* seconds */
    time_t next_collection;
    
    /* Retention policy */
    uint32_t retention_days;
    bool auto_cleanup;
    
    pthread_mutex_t lock;
} metric_t;

/* Alert condition */
typedef struct {
    uint32_t condition_id;
    char expression[512];    /* Boolean expression */
    uint32_t metric_id;
    double threshold_value;
    char comparison[8];      /* >, <, >=, <=, ==, != */
    uint32_t duration;      /* Seconds condition must persist */
    time_t first_triggered;
    bool active;
} alert_condition_t;

/* Alert definition */
typedef struct {
    uint32_t alert_id;
    char name[128];
    char description[512];
    alert_severity_t severity;
    
    /* Conditions */
    alert_condition_t conditions[10];
    uint32_t condition_count;
    
    /* Notification settings */
    bool email_enabled;
    char email_recipients[512];
    bool sms_enabled;
    char sms_numbers[256];
    bool webhook_enabled;
    char webhook_url[512];
    
    /* State */
    bool enabled;
    bool currently_active;
    time_t last_triggered;
    uint32_t trigger_count;
    
    /* Escalation */
    uint32_t escalation_delay; /* minutes */
    alert_severity_t escalation_severity;
    
    pthread_mutex_t lock;
} alert_t;

/* Performance prediction */
typedef struct {
    uint32_t prediction_id;
    uint32_t metric_id;
    prediction_model_t model;
    
    /* Prediction parameters */
    uint32_t forecast_horizon; /* hours */
    uint32_t training_window;  /* days */
    double confidence_interval;
    
    /* Results */
    double predicted_value;
    double confidence_upper;
    double confidence_lower;
    time_t prediction_time;
    time_t valid_until;
    
    /* Model performance */
    double accuracy_score;
    double mean_absolute_error;
    double root_mean_square_error;
    
    /* Configuration */
    bool enabled;
    time_t last_trained;
    uint32_t retrain_interval; /* hours */
    
    pthread_mutex_t lock;
} prediction_t;

/* System health assessment */
typedef struct {
    uint32_t health_score;   /* 0-100 overall health */
    
    /* Component health scores */
    uint32_t cpu_health;
    uint32_t memory_health;
    uint32_t disk_health;
    uint32_t network_health;
    uint32_t security_health;
    uint32_t application_health;
    
    /* Risk assessment */
    uint32_t failure_risk;   /* 0-100 probability of failure */
    char risk_factors[1024];
    time_t next_maintenance;
    
    /* Recommendations */
    char recommendations[2048];
    uint32_t recommendation_count;
    
    time_t last_assessment;
    bool assessment_valid;
} health_assessment_t;

/* Dashboard widget */
typedef struct {
    uint32_t widget_id;
    char name[128];
    char type[64];          /* chart, gauge, table, text */
    
    /* Data source */
    uint32_t metric_ids[20];
    uint32_t metric_count;
    
    /* Display settings */
    uint32_t time_range;    /* seconds */
    aggregation_method_t aggregation;
    uint32_t refresh_interval; /* seconds */
    
    /* Position and size */
    uint32_t x, y, width, height;
    
    /* Configuration */
    char config_json[2048];
    bool enabled;
} dashboard_widget_t;

/* Dashboard */
typedef struct {
    uint32_t dashboard_id;
    char name[128];
    char description[512];
    
    /* Widgets */
    dashboard_widget_t widgets[50];
    uint32_t widget_count;
    
    /* Access control */
    char owner[64];
    char permissions[256];
    bool public_access;
    
    /* Settings */
    uint32_t auto_refresh; /* seconds */
    time_t created;
    time_t last_modified;
    
    pthread_mutex_t lock;
} dashboard_t;

/* Service monitoring */
typedef struct {
    uint32_t service_id;
    char name[128];
    char description[256];
    
    /* Service details */
    pid_t process_id;
    char executable_path[512];
    char config_file[512];
    
    /* Status */
    bool running;
    bool healthy;
    time_t start_time;
    time_t last_check;
    
    /* Performance metrics */
    double cpu_usage;
    uint64_t memory_usage;
    uint32_t thread_count;
    uint64_t disk_io;
    uint64_t network_io;
    
    /* Health check */
    char health_check_url[256];
    uint32_t health_check_interval; /* seconds */
    uint32_t health_timeout;
    uint32_t consecutive_failures;
    
    /* Restart policy */
    bool auto_restart;
    uint32_t max_restarts;
    uint32_t restart_delay; /* seconds */
    
    pthread_mutex_t lock;
} service_monitor_t;

/* Anomaly detection */
typedef struct {
    uint32_t anomaly_id;
    uint32_t metric_id;
    
    /* Detection parameters */
    double sensitivity;     /* 0.0 - 1.0 */
    uint32_t window_size;   /* data points */
    double threshold_factor; /* standard deviations */
    
    /* Model state */
    double baseline_mean;
    double baseline_stddev;
    double *seasonal_factors;
    uint32_t seasonal_period;
    
    /* Detection results */
    bool anomaly_detected;
    double anomaly_score;
    time_t detection_time;
    char description[256];
    
    /* Configuration */
    bool enabled;
    time_t last_trained;
    uint32_t retrain_interval; /* hours */
    
    pthread_mutex_t lock;
} anomaly_detector_t;

/* Monitoring system configuration */
typedef struct {
    /* Collection settings */
    uint32_t default_interval;    /* seconds */
    uint32_t max_data_points;
    uint32_t retention_days;
    
    /* Performance settings */
    uint32_t collection_threads;
    uint32_t processing_threads;
    uint32_t max_memory_mb;
    
    /* Storage settings */
    char storage_path[512];
    bool compress_data;
    uint32_t compression_level;
    
    /* Network settings */
    char metrics_port[16];
    char api_port[16];
    bool ssl_enabled;
    char ssl_cert_path[512];
    
    /* AI/ML settings */
    bool predictions_enabled;
    bool anomaly_detection_enabled;
    uint32_t model_update_interval; /* hours */
    
    /* Alerting settings */
    char smtp_server[256];
    char smtp_username[128];
    char smtp_password[128];
    char webhook_timeout[16];
    
    /* Dashboard settings */
    uint32_t max_dashboards;
    uint32_t default_refresh;
    bool public_dashboards;
    
} monitoring_config_t;

/* Main monitoring system */
typedef struct {
    /* Core components */
    metric_t metrics[MAX_METRICS];
    uint32_t metric_count;
    
    alert_t alerts[MAX_ALERTS];
    uint32_t alert_count;
    
    prediction_t predictions[MAX_PREDICTIONS];
    uint32_t prediction_count;
    
    dashboard_t dashboards[MAX_DASHBOARDS];
    uint32_t dashboard_count;
    
    service_monitor_t services[MAX_SERVICES];
    uint32_t service_count;
    
    anomaly_detector_t detectors[MAX_METRICS];
    uint32_t detector_count;
    
    /* System state */
    health_assessment_t health;
    monitoring_config_t config;
    
    /* Threading */
    pthread_t collection_threads[16];
    pthread_t processing_thread;
    pthread_t alert_thread;
    pthread_t prediction_thread;
    pthread_t health_thread;
    bool threads_running;
    
    /* Synchronization */
    pthread_mutex_t system_lock;
    pthread_cond_t data_available;
    
    /* Statistics */
    uint64_t metrics_collected;
    uint64_t alerts_triggered;
    uint64_t predictions_made;
    time_t start_time;
    
    bool initialized;
} monitoring_system_t;

/* Core functions */
int monitoring_system_init(void);
int monitoring_system_cleanup(void);
int monitoring_load_config(const char *config_file);

/* Metric management */
int metric_create(const char *name, metric_type_t type, data_source_t source);
int metric_update(uint32_t metric_id, double value);
int metric_get_current(uint32_t metric_id, double *value);
int metric_get_history(uint32_t metric_id, metric_point_t *points, uint32_t count);
int metric_set_threshold(uint32_t metric_id, double warning, double critical);
int metric_enable_collection(uint32_t metric_id, bool enabled);

/* Alert management */
int alert_create(const char *name, alert_severity_t severity);
int alert_add_condition(uint32_t alert_id, uint32_t metric_id, const char *comparison, double threshold);
int alert_set_notification(uint32_t alert_id, const char *email, const char *webhook);
int alert_check_conditions(uint32_t alert_id);
int alert_trigger(uint32_t alert_id, const char *message);

/* Prediction system */
int prediction_create(uint32_t metric_id, prediction_model_t model);
int prediction_train_model(uint32_t prediction_id);
int prediction_forecast(uint32_t prediction_id, uint32_t horizon_hours);
int prediction_get_result(uint32_t prediction_id, double *value, double *confidence);

/* Dashboard management */
int dashboard_create(const char *name, const char *description);
int dashboard_add_widget(uint32_t dashboard_id, const char *name, const char *type);
int dashboard_configure_widget(uint32_t dashboard_id, uint32_t widget_id, const char *config_json);
int dashboard_export(uint32_t dashboard_id, const char *format, const char *output_path);

/* Service monitoring */
int service_monitor_add(const char *name, const char *executable_path);
int service_monitor_remove(uint32_t service_id);
int service_check_health(uint32_t service_id);
int service_restart(uint32_t service_id);
int service_get_metrics(uint32_t service_id, service_monitor_t *metrics);

/* Health assessment */
int health_assess_system(health_assessment_t *assessment);
int health_predict_failures(time_t *next_failure, char *components);
int health_get_recommendations(char *recommendations, size_t max_size);

/* Anomaly detection */
int anomaly_detector_create(uint32_t metric_id, double sensitivity);
int anomaly_detector_train(uint32_t detector_id);
int anomaly_check_metric(uint32_t metric_id, bool *anomaly_detected, double *score);

/* Query and analysis */
int query_metrics(const char *query, metric_point_t *results, uint32_t max_results);
int analyze_performance_trend(uint32_t metric_id, const char *analysis_type, char *result);
int generate_report(const char *report_type, const char *output_path);

/* Data management */
int data_export(const char *format, const char *output_path);
int data_import(const char *input_path);
int data_cleanup_old(uint32_t retention_days);
int data_compress_archive(const char *archive_path);

/* API functions */
int api_server_start(uint16_t port);
int api_server_stop(void);
int webhook_send_notification(const char *url, const char *payload);

/* Utility functions */
const char *metric_type_name(metric_type_t type);
const char *data_source_name(data_source_t source);
const char *alert_severity_name(alert_severity_t severity);
const char *prediction_model_name(prediction_model_t model);

#endif /* ENTERPRISE_MONITORING_H */