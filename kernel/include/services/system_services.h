/*
 * LimitlessOS Advanced System Services
 * Systemd-compatible service management, logging, monitoring, and maintenance
 */

#ifndef SYSTEM_SERVICES_H
#define SYSTEM_SERVICES_H

#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <pthread.h>
#include <sys/types.h>

/* Service states */
typedef enum {
    SERVICE_STATE_INACTIVE,     /* Service not running */
    SERVICE_STATE_ACTIVE,       /* Service running */
    SERVICE_STATE_ACTIVATING,   /* Service starting */
    SERVICE_STATE_DEACTIVATING, /* Service stopping */
    SERVICE_STATE_FAILED,       /* Service failed */
    SERVICE_STATE_RELOADING,    /* Service reloading config */
    SERVICE_STATE_MAINTENANCE   /* Service in maintenance mode */
} service_state_t;

/* Service types */
typedef enum {
    SERVICE_TYPE_SIMPLE,        /* Simple daemon process */
    SERVICE_TYPE_FORKING,       /* Traditional forking daemon */
    SERVICE_TYPE_ONESHOT,       /* One-shot service */
    SERVICE_TYPE_NOTIFY,        /* Service sends readiness notification */
    SERVICE_TYPE_DBUS,          /* D-Bus activated service */
    SERVICE_TYPE_IDLE,          /* Idle service (started when system idle) */
    SERVICE_TYPE_EXEC,          /* Execute and exit */
    SERVICE_TYPE_SOCKET         /* Socket-activated service */
} service_type_t;

/* Service restart policies */
typedef enum {
    RESTART_NO,                 /* Never restart */
    RESTART_ON_SUCCESS,         /* Restart only on success */
    RESTART_ON_FAILURE,         /* Restart only on failure */
    RESTART_ON_ABNORMAL,        /* Restart on abnormal exit */
    RESTART_ON_WATCHDOG,        /* Restart on watchdog timeout */
    RESTART_ON_ABORT,           /* Restart on abort signal */
    RESTART_ALWAYS              /* Always restart */
} service_restart_t;

/* Log levels */
typedef enum {
    LOG_LEVEL_EMERGENCY = 0,    /* System is unusable */
    LOG_LEVEL_ALERT     = 1,    /* Action must be taken immediately */
    LOG_LEVEL_CRITICAL  = 2,    /* Critical conditions */
    LOG_LEVEL_ERROR     = 3,    /* Error conditions */
    LOG_LEVEL_WARNING   = 4,    /* Warning conditions */
    LOG_LEVEL_NOTICE    = 5,    /* Normal but significant condition */
    LOG_LEVEL_INFO      = 6,    /* Informational messages */
    LOG_LEVEL_DEBUG     = 7     /* Debug-level messages */
} log_level_t;

/* Service configuration */
typedef struct {
    char name[64];              /* Service name */
    char description[256];      /* Service description */
    service_type_t type;        /* Service type */
    
    /* Execution */
    char exec_start[512];       /* Start command */
    char exec_stop[512];        /* Stop command */
    char exec_reload[512];      /* Reload command */
    char working_directory[256]; /* Working directory */
    char user[32];              /* Run as user */
    char group[32];             /* Run as group */
    
    /* Environment */
    char environment[32][256];  /* Environment variables */
    uint32_t environment_count;
    char environment_file[256]; /* Environment file */
    
    /* Dependencies */
    char wants[16][64];         /* Weak dependencies */
    uint32_t wants_count;
    char requires[16][64];      /* Strong dependencies */
    uint32_t requires_count;
    char conflicts[16][64];     /* Conflicting services */
    uint32_t conflicts_count;
    char after[16][64];         /* Start after these services */
    uint32_t after_count;
    char before[16][64];        /* Start before these services */
    uint32_t before_count;
    
    /* Restart configuration */
    service_restart_t restart;
    uint32_t restart_sec;       /* Restart delay in seconds */
    uint32_t start_limit_interval; /* Start limit window */
    uint32_t start_limit_burst; /* Max starts in window */
    
    /* Timeouts */
    uint32_t timeout_start_sec; /* Start timeout */
    uint32_t timeout_stop_sec;  /* Stop timeout */
    uint32_t timeout_abort_sec; /* Abort timeout */
    
    /* Resource limits */
    uint64_t memory_limit;      /* Memory limit in bytes */
    uint32_t cpu_quota;         /* CPU quota percentage */
    uint64_t disk_quota;        /* Disk quota in bytes */
    uint32_t tasks_max;         /* Maximum number of tasks */
    
    /* Security */
    bool private_tmp;           /* Private /tmp */
    bool private_network;       /* Private network namespace */
    bool no_new_privileges;     /* Disable privilege escalation */
    bool protect_system;        /* Protect system directories */
    bool protect_home;          /* Protect home directories */
    char capabilities[256];     /* Linux capabilities */
    
    /* Monitoring */
    bool watchdog_enabled;      /* Enable watchdog */
    uint32_t watchdog_sec;      /* Watchdog timeout */
    
    /* Socket activation */
    struct {
        bool enabled;
        char listen_stream[256]; /* TCP/Unix socket */
        char listen_dgram[256];  /* UDP socket */
        uint32_t backlog;       /* Socket backlog */
        bool accept;            /* Accept connections */
        uint32_t max_connections; /* Max concurrent connections */
    } socket;
} service_config_t;

/* Service runtime information */
typedef struct {
    service_config_t config;
    service_state_t state;
    pid_t main_pid;             /* Main process PID */
    pid_t control_pid;          /* Control process PID */
    
    /* Timestamps */
    time_t load_time;           /* When service was loaded */
    time_t active_enter_time;   /* When service became active */
    time_t active_exit_time;    /* When service became inactive */
    time_t state_change_time;   /* Last state change */
    
    /* Statistics */
    uint64_t restart_count;     /* Number of restarts */
    uint64_t exec_start_count;  /* Number of start attempts */
    uint64_t memory_current;    /* Current memory usage */
    double cpu_usage_nsec;      /* CPU usage in nanoseconds */
    
    /* Status */
    int exit_code;              /* Last exit code */
    int exit_status;            /* Last exit status */
    char status_text[256];      /* Status message */
    
    /* Control */
    pthread_mutex_t mutex;
    int notify_fd;              /* Notification file descriptor */
    bool stop_requested;        /* Stop requested flag */
    
    /* Callbacks */
    void (*state_change_callback)(const char *service_name, service_state_t new_state);
} service_t;

/* Log entry */
typedef struct {
    uint64_t timestamp;         /* Nanoseconds since epoch */
    log_level_t level;          /* Log level */
    char facility[32];          /* Log facility */
    char service_name[64];      /* Service name */
    pid_t pid;                  /* Process ID */
    uint32_t tid;               /* Thread ID */
    char hostname[64];          /* Hostname */
    char message[1024];         /* Log message */
    
    /* Structured data */
    struct {
        char key[64];
        char value[256];
    } fields[32];
    uint32_t field_count;
    
    /* Context */
    char boot_id[37];           /* Boot ID UUID */
    char machine_id[37];        /* Machine ID UUID */
    uint64_t monotonic_usec;    /* Monotonic timestamp */
    uint64_t realtime_usec;     /* Realtime timestamp */
} log_entry_t;

/* System monitoring metrics */
typedef struct {
    uint64_t timestamp;
    
    /* CPU metrics */
    double cpu_usage_percent;   /* Overall CPU usage */
    double cpu_load_1min;       /* 1-minute load average */
    double cpu_load_5min;       /* 5-minute load average */
    double cpu_load_15min;      /* 15-minute load average */
    uint32_t cpu_context_switches; /* Context switches per second */
    uint32_t cpu_interrupts;    /* Interrupts per second */
    
    /* Memory metrics */
    uint64_t memory_total;      /* Total memory */
    uint64_t memory_available;  /* Available memory */
    uint64_t memory_used;       /* Used memory */
    uint64_t memory_cached;     /* Cached memory */
    uint64_t memory_buffers;    /* Buffer memory */
    uint64_t swap_total;        /* Total swap */
    uint64_t swap_used;         /* Used swap */
    
    /* Storage metrics */
    uint64_t disk_read_bytes;   /* Bytes read per second */
    uint64_t disk_write_bytes;  /* Bytes written per second */
    uint64_t disk_read_ops;     /* Read operations per second */
    uint64_t disk_write_ops;    /* Write operations per second */
    double disk_utilization;    /* Disk utilization percentage */
    
    /* Network metrics */
    uint64_t network_rx_bytes;  /* Bytes received per second */
    uint64_t network_tx_bytes;  /* Bytes transmitted per second */
    uint64_t network_rx_packets; /* Packets received per second */
    uint64_t network_tx_packets; /* Packets transmitted per second */
    uint32_t network_connections; /* Active connections */
    
    /* System metrics */
    uint32_t processes_total;   /* Total processes */
    uint32_t processes_running; /* Running processes */
    uint32_t processes_sleeping; /* Sleeping processes */
    uint32_t processes_zombie;  /* Zombie processes */
    double uptime;              /* System uptime in seconds */
    double temperature;         /* System temperature in Celsius */
    double power_usage;         /* Power usage in watts */
} system_metrics_t;

/* Service manager */
typedef struct {
    bool initialized;
    
    /* Service management */
    service_t services[512];    /* Up to 512 services */
    uint32_t service_count;
    pthread_mutex_t services_mutex;
    
    /* Configuration */
    char config_dir[256];       /* Configuration directory */
    char runtime_dir[256];      /* Runtime directory */
    char log_dir[256];          /* Log directory */
    
    /* Logging system */
    struct {
        bool enabled;
        log_level_t min_level;  /* Minimum log level */
        bool structured_logging; /* Enable structured logging */
        bool persistent_logging; /* Persistent storage */
        uint64_t max_log_size;  /* Maximum log file size */
        uint32_t max_log_files; /* Maximum log files to keep */
        char log_format[256];   /* Log format string */
        
        /* Log storage */
        log_entry_t *entries;   /* Log entry buffer */
        uint64_t entry_count;
        uint64_t entry_index;
        uint64_t max_entries;
        pthread_mutex_t log_mutex;
        
        /* Syslog integration */
        bool syslog_enabled;
        char syslog_facility[32];
        char syslog_identifier[64];
    } logging;
    
    /* System monitoring */
    struct {
        bool enabled;
        uint32_t collection_interval; /* Collection interval in seconds */
        system_metrics_t current;   /* Current metrics */
        system_metrics_t *history;  /* Historical metrics */
        uint32_t history_size;
        uint32_t history_index;
        pthread_mutex_t metrics_mutex;
        pthread_t monitor_thread;
        
        /* Alerting */
        struct {
            bool enabled;
            double cpu_threshold;    /* CPU usage alert threshold */
            double memory_threshold; /* Memory usage alert threshold */
            double disk_threshold;   /* Disk usage alert threshold */
            uint32_t load_threshold; /* Load average threshold */
            void (*alert_callback)(const char *alert_type, const char *message);
        } alerting;
    } monitoring;
    
    /* Automatic maintenance */
    struct {
        bool enabled;
        uint32_t maintenance_hour;   /* Hour to run maintenance (0-23) */
        bool log_rotation;          /* Enable log rotation */
        bool cache_cleanup;         /* Enable cache cleanup */
        bool package_updates;       /* Enable automatic updates */
        bool system_optimization;   /* Enable system optimization */
        uint64_t last_maintenance;  /* Last maintenance timestamp */
        
        /* Statistics */
        uint64_t maintenance_runs;
        uint64_t logs_rotated;
        uint64_t cache_cleaned_mb;
        uint64_t packages_updated;
    } maintenance;
    
    /* D-Bus integration */
    struct {
        bool enabled;
        char bus_name[256];         /* D-Bus service name */
        char object_path[256];      /* D-Bus object path */
        void *connection;           /* D-Bus connection */
    } dbus;
    
    /* Statistics */
    struct {
        uint64_t services_started;
        uint64_t services_stopped;
        uint64_t services_failed;
        uint64_t services_restarted;
        uint64_t log_entries_written;
        uint64_t monitoring_cycles;
        uint64_t alerts_triggered;
    } stats;
} service_manager_t;

/* Function prototypes */

/* Service manager initialization */
int service_manager_init(void);
void service_manager_exit(void);
int service_manager_reload(void);

/* Service management */
int service_load(const char *service_file);
int service_unload(const char *service_name);
int service_start(const char *service_name);
int service_stop(const char *service_name);
int service_restart(const char *service_name);
int service_reload_config(const char *service_name);
int service_enable(const char *service_name);
int service_disable(const char *service_name);

/* Service information */
service_state_t service_get_state(const char *service_name);
int service_get_info(const char *service_name, service_t *info);
int service_list(char service_names[][64], int max_services);
int service_list_by_state(service_state_t state, char service_names[][64], int max_services);
bool service_is_active(const char *service_name);
bool service_is_enabled(const char *service_name);

/* Service configuration */
int service_config_load(const char *config_file, service_config_t *config);
int service_config_save(const service_config_t *config, const char *config_file);
int service_config_validate(const service_config_t *config);
int service_set_environment(const char *service_name, const char *key, const char *value);

/* Logging system */
int logging_init(void);
int logging_write(log_level_t level, const char *facility, const char *service_name, 
                 const char *format, ...);
int logging_write_structured(log_level_t level, const char *facility, 
                           const char *service_name, const char *message,
                           const char *fields[][2], uint32_t field_count);
int logging_query(log_level_t min_level, const char *service_filter, 
                 time_t start_time, time_t end_time, 
                 log_entry_t *entries, uint32_t max_entries);
int logging_rotate(void);
int logging_export(const char *filename, const char *format);

/* System monitoring */
int monitoring_init(void);
int monitoring_start(void);
int monitoring_stop(void);
int monitoring_get_metrics(system_metrics_t *metrics);
int monitoring_get_history(system_metrics_t *history, uint32_t max_entries);
int monitoring_set_alert_threshold(const char *metric, double threshold);
int monitoring_enable_alert(const char *alert_type);
int monitoring_disable_alert(const char *alert_type);

/* Automatic maintenance */
int maintenance_init(void);
int maintenance_run(void);
int maintenance_schedule(uint32_t hour);
int maintenance_enable_feature(const char *feature);
int maintenance_disable_feature(const char *feature);
int maintenance_get_stats(void);

/* Socket activation */
int socket_activation_init(void);
int socket_create_listener(const char *service_name, const service_config_t *config);
int socket_accept_connection(const char *service_name);
int socket_close_listener(const char *service_name);

/* D-Bus integration */
int dbus_init(void);
int dbus_register_service(const char *service_name);
int dbus_unregister_service(const char *service_name);
int dbus_send_signal(const char *signal_name, const char *data);

/* Systemd compatibility */
int systemctl_daemon_reload(void);
int systemctl_start(const char *service_name);
int systemctl_stop(const char *service_name);
int systemctl_restart(const char *service_name);
int systemctl_enable(const char *service_name);
int systemctl_disable(const char *service_name);
int systemctl_status(const char *service_name);
int systemctl_list_units(void);

/* Utility functions */
const char *service_state_name(service_state_t state);
const char *service_type_name(service_type_t type);
const char *log_level_name(log_level_t level);
const char *restart_policy_name(service_restart_t restart);
int parse_service_file(const char *filename, service_config_t *config);
int generate_service_file(const service_config_t *config, const char *filename);

/* Global service manager */
extern service_manager_t service_manager;

#endif /* SYSTEM_SERVICES_H */