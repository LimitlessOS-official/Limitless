#ifndef LIMITLESS_AUDIT_LOG_H
#define LIMITLESS_AUDIT_LOG_H

#include "types.h"

/* Maximum log entries in the ring buffer */
#define AUDIT_LOG_MAX_ENTRIES 8192
#define AUDIT_LOG_ENTRY_MAX_SIZE 512

/* Event types */
typedef enum {
    AUDIT_EVENT_FILE_ACCESS = 1,
    AUDIT_EVENT_PROCESS_CREATE = 2,
    AUDIT_EVENT_PROCESS_EXIT = 3,
    AUDIT_EVENT_NETWORK_CONNECT = 4,
    AUDIT_EVENT_SYSCALL = 5,
    AUDIT_EVENT_LOGIN = 6,
    AUDIT_EVENT_LOGOUT = 7,
    AUDIT_EVENT_PRIVILEGE_USE = 8,
    AUDIT_EVENT_POLICY_CHANGE = 9,
    AUDIT_EVENT_AUTHENTICATION = 10,
    AUDIT_EVENT_AUTHORIZATION = 11,
    AUDIT_EVENT_CRYPTO_OPERATION = 12,
    AUDIT_EVENT_BACKUP_OPERATION = 13,
    AUDIT_EVENT_COMPLIANCE_VIOLATION = 14,
    AUDIT_EVENT_SECURITY_ALERT = 15
} audit_event_type_t;

/* Event severity levels */
typedef enum {
    AUDIT_SEVERITY_INFO = 0,
    AUDIT_SEVERITY_WARNING = 1,
    AUDIT_SEVERITY_ERROR = 2,
    AUDIT_SEVERITY_CRITICAL = 3
} audit_severity_t;

/* Audit event structure */
typedef struct {
    uint64_t timestamp;          /* Event timestamp */
    uint64_t sequence_number;    /* Monotonic sequence number */
    audit_event_type_t type;     /* Event type */
    audit_severity_t severity;   /* Event severity */
    pid_t process_id;           /* Process that triggered event */
    uint32_t user_id;           /* User ID */
    uint32_t group_id;          /* Group ID */
    uint32_t session_id;        /* Session ID */
    char process_name[64];      /* Process name */
    char event_data[256];       /* Event-specific data */
    uint32_t data_length;       /* Length of event_data */
    uint32_t checksum;          /* Integrity checksum */
} audit_log_entry_t;

/* Ring buffer structure */
typedef struct {
    audit_log_entry_t entries[AUDIT_LOG_MAX_ENTRIES];
    uint32_t head;              /* Write position */
    uint32_t tail;              /* Read position */
    uint32_t count;             /* Number of entries */
    uint64_t next_sequence;     /* Next sequence number */
    spinlock_t lock;            /* Synchronization */
    bool overflow;              /* Buffer has overflowed */
    uint64_t total_events;      /* Total events logged */
    uint64_t dropped_events;    /* Events dropped due to overflow */
} audit_ring_buffer_t;

/* Audit configuration */
typedef struct {
    bool enabled;               /* Audit logging enabled */
    bool log_file_access;       /* Log file access events */
    bool log_process_events;    /* Log process create/exit */
    bool log_network_events;    /* Log network connections */
    bool log_syscalls;          /* Log system calls */
    bool log_authentication;    /* Log authentication events */
    bool log_authorization;     /* Log authorization failures */
    bool log_crypto_ops;        /* Log cryptographic operations */
    bool auto_rotate;           /* Auto-rotate when full */
    uint32_t max_file_size;     /* Max size before rotation */
    char log_file_path[256];    /* Path to persistent log file */
    audit_severity_t min_severity; /* Minimum severity to log */
} audit_config_t;

/* Persistent log file header */
typedef struct {
    uint32_t magic;             /* Magic number for validation */
    uint32_t version;           /* Log format version */
    uint64_t created_time;      /* Log creation timestamp */
    uint64_t last_write_time;   /* Last write timestamp */
    uint32_t entry_count;       /* Number of entries in file */
    uint32_t file_size;         /* Current file size */
    uint32_t checksum;          /* File integrity checksum */
    uint8_t reserved[32];       /* Reserved for future use */
} audit_log_header_t;

/* Statistics structure */
typedef struct {
    uint64_t events_logged;     /* Total events logged */
    uint64_t events_dropped;    /* Events dropped */
    uint64_t file_writes;       /* Number of file writes */
    uint64_t file_rotations;    /* Number of log rotations */
    uint64_t integrity_failures; /* Integrity check failures */
    uint64_t last_rotation_time; /* Last rotation timestamp */
    uint32_t current_file_size;  /* Current log file size */
} audit_statistics_t;

/* Search criteria for log queries */
typedef struct {
    uint64_t start_time;        /* Start timestamp (0 = no limit) */
    uint64_t end_time;          /* End timestamp (0 = no limit) */
    audit_event_type_t event_type; /* Event type (0 = all types) */
    audit_severity_t min_severity;  /* Minimum severity */
    pid_t process_id;           /* Process ID (0 = all processes) */
    uint32_t user_id;           /* User ID (0xFFFFFFFF = all users) */
    char process_name[64];      /* Process name pattern */
    char event_data_pattern[128]; /* Data pattern to search for */
    uint32_t max_results;       /* Maximum results to return */
} audit_search_criteria_t;

/* Function declarations */
status_t audit_init(void);
status_t audit_shutdown(void);
status_t audit_configure(const audit_config_t* config);
status_t audit_get_config(audit_config_t* out_config);

/* Event logging */
status_t audit_log_event(audit_event_type_t type, audit_severity_t severity,
                        pid_t pid, const char* format, ...);
status_t audit_log_file_access(pid_t pid, const char* path, uint32_t access_type, bool granted);
status_t audit_log_process_create(pid_t parent_pid, pid_t child_pid, const char* program);
status_t audit_log_process_exit(pid_t pid, int exit_code);
status_t audit_log_network_connect(pid_t pid, uint32_t remote_addr, uint16_t port, bool success);
status_t audit_log_authentication(const char* username, bool success, const char* method);
status_t audit_log_authorization(pid_t pid, const char* resource, bool granted);
status_t audit_log_syscall(pid_t pid, uint32_t syscall_number, const char* syscall_name);
status_t audit_log_policy_change(const char* policy_type, const char* description);
status_t audit_log_crypto_operation(pid_t pid, const char* operation, bool success);

/* Log retrieval */
status_t audit_get_entries(audit_log_entry_t* out_entries, uint32_t* in_out_count);
status_t audit_search_entries(const audit_search_criteria_t* criteria,
                             audit_log_entry_t* out_entries, uint32_t* in_out_count);
status_t audit_get_recent_entries(uint32_t count, audit_log_entry_t* out_entries, uint32_t* out_actual_count);

/* Log management */
status_t audit_flush_to_disk(void);
status_t audit_rotate_log(void);
status_t audit_clear_log(void);
status_t audit_export_log(const char* export_path);
status_t audit_import_log(const char* import_path);

/* Statistics and monitoring */
status_t audit_get_statistics(audit_statistics_t* out_stats);
status_t audit_reset_statistics(void);
bool audit_is_buffer_full(void);
float audit_get_buffer_usage(void);

/* Integrity verification */
status_t audit_verify_integrity(bool* out_valid);
status_t audit_rebuild_checksums(void);

/* Userspace interface */
status_t audit_syscall_handler(uint32_t cmd, void* arg);

/* Syscall commands */
#define AUDIT_CMD_GET_CONFIG      1
#define AUDIT_CMD_SET_CONFIG      2
#define AUDIT_CMD_GET_ENTRIES     3
#define AUDIT_CMD_SEARCH_ENTRIES  4
#define AUDIT_CMD_GET_STATISTICS  5
#define AUDIT_CMD_FLUSH_LOG       6
#define AUDIT_CMD_ROTATE_LOG      7
#define AUDIT_CMD_CLEAR_LOG       8
#define AUDIT_CMD_VERIFY_INTEGRITY 9

/* File format constants */
#define AUDIT_LOG_MAGIC     0x4C4F4741  /* "LOGL" */
#define AUDIT_LOG_VERSION   1
#define AUDIT_MAX_FILE_SIZE (64 * 1024 * 1024)  /* 64MB default max */

#endif /* LIMITLESS_AUDIT_LOG_H */