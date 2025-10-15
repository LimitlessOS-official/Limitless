/*
 * LimitlessOS System Recovery & Backup Framework
 * Comprehensive recovery, rollback, and backup solutions
 */

#ifndef LIMITLESSOS_SYSTEM_RECOVERY_H
#define LIMITLESSOS_SYSTEM_RECOVERY_H

#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <pthread.h>

/* Recovery System Types */
typedef enum {
    RECOVERY_TYPE_SYSTEM = 0,       /* Full system recovery */
    RECOVERY_TYPE_BOOT,             /* Boot environment recovery */
    RECOVERY_TYPE_USER_DATA,        /* User data recovery */
    RECOVERY_TYPE_APPLICATION,      /* Application-specific recovery */
    RECOVERY_TYPE_CONFIGURATION,    /* System configuration recovery */
    RECOVERY_TYPE_KERNEL,           /* Kernel recovery */
    RECOVERY_TYPE_MAX
} recovery_type_t;

/* Backup Types */
typedef enum {
    BACKUP_TYPE_FULL = 0,          /* Complete system backup */
    BACKUP_TYPE_INCREMENTAL,       /* Changes since last backup */
    BACKUP_TYPE_DIFFERENTIAL,      /* Changes since last full backup */
    BACKUP_TYPE_SNAPSHOT,          /* Instant point-in-time snapshot */
    BACKUP_TYPE_CONTINUOUS,        /* Real-time continuous backup */
    BACKUP_TYPE_MAX
} backup_type_t;

/* Recovery Modes */
typedef enum {
    RECOVERY_MODE_AUTOMATIC = 0,   /* Automatic recovery */
    RECOVERY_MODE_INTERACTIVE,     /* User-guided recovery */
    RECOVERY_MODE_EMERGENCY,       /* Emergency recovery mode */
    RECOVERY_MODE_SAFE,           /* Safe mode recovery */
    RECOVERY_MODE_MINIMAL,        /* Minimal recovery mode */
    RECOVERY_MODE_MAX
} recovery_mode_t;

/* Backup Storage Types */
typedef enum {
    BACKUP_STORAGE_LOCAL = 0,      /* Local storage */
    BACKUP_STORAGE_NETWORK,        /* Network storage */
    BACKUP_STORAGE_CLOUD,          /* Cloud storage */
    BACKUP_STORAGE_REMOVABLE,      /* Removable media */
    BACKUP_STORAGE_MAX
} backup_storage_t;

/* Compression Types */
typedef enum {
    COMPRESSION_NONE = 0,
    COMPRESSION_GZIP,
    COMPRESSION_BZIP2,
    COMPRESSION_XZ,
    COMPRESSION_LZ4,
    COMPRESSION_ZSTD,
    COMPRESSION_MAX
} compression_type_t;

/* Encryption Types */
typedef enum {
    ENCRYPTION_NONE = 0,
    ENCRYPTION_AES128,
    ENCRYPTION_AES256,
    ENCRYPTION_CHACHA20,
    ENCRYPTION_MAX
} encryption_type_t;

/* Recovery Point Structure */
typedef struct {
    char id[64];                   /* Unique recovery point ID */
    char name[128];               /* Human-readable name */
    char description[256];        /* Description of recovery point */
    time_t timestamp;             /* Creation timestamp */
    recovery_type_t type;         /* Type of recovery point */
    backup_type_t backup_type;    /* Backup type used */
    
    /* Size and checksums */
    uint64_t total_size;          /* Total backup size */
    uint64_t compressed_size;     /* Compressed size */
    char checksum[65];            /* SHA-256 checksum */
    
    /* Metadata */
    uint32_t version;             /* Recovery point version */
    uint32_t kernel_version;      /* Kernel version at backup time */
    char os_version[32];          /* OS version string */
    bool bootable;                /* Can be used for boot recovery */
    bool verified;                /* Integrity verified */
    
    /* Storage information */
    backup_storage_t storage_type;
    char storage_path[512];       /* Storage location */
    compression_type_t compression;
    encryption_type_t encryption;
    
    /* Dependencies */
    char parent_id[64];           /* Parent recovery point (for incremental) */
    uint32_t dependency_count;    /* Number of dependencies */
    char dependencies[16][64];    /* Dependency recovery point IDs */
} recovery_point_t;

/* Backup Configuration */
typedef struct {
    bool enabled;                 /* Backup system enabled */
    uint32_t retention_days;      /* Backup retention period */
    uint32_t max_recovery_points; /* Maximum recovery points to keep */
    
    /* Schedule settings */
    bool scheduled_backup;        /* Enable scheduled backups */
    uint32_t backup_interval_hours; /* Backup interval in hours */
    uint32_t backup_time_hour;    /* Daily backup time (0-23) */
    uint32_t weekly_backup_day;   /* Weekly backup day (0-6, Sunday=0) */
    
    /* Backup paths */
    char backup_root[512];        /* Root backup directory */
    char temp_dir[512];           /* Temporary directory for operations */
    
    /* Include/exclude patterns */
    uint32_t include_count;
    char include_patterns[64][256];
    uint32_t exclude_count;
    char exclude_patterns[64][256];
    
    /* Compression and encryption */
    compression_type_t default_compression;
    encryption_type_t default_encryption;
    char encryption_key[256];     /* Base64-encoded encryption key */
    
    /* Storage configuration */
    backup_storage_t primary_storage;
    backup_storage_t secondary_storage;
    char network_location[512];   /* Network backup location */
    char cloud_endpoint[512];     /* Cloud backup endpoint */
    
    /* Performance settings */
    uint32_t max_backup_threads;  /* Maximum backup threads */
    uint32_t throttle_bandwidth_mbps; /* Bandwidth throttling */
    bool low_priority_mode;       /* Use low CPU/IO priority */
    
    /* Verification settings */
    bool verify_backups;          /* Verify backup integrity */
    bool test_restore;            /* Test restore functionality */
    uint32_t verification_interval_days; /* Verification frequency */
} backup_config_t;

/* Recovery Status */
typedef enum {
    RECOVERY_STATUS_IDLE = 0,
    RECOVERY_STATUS_SCANNING,
    RECOVERY_STATUS_CREATING_BACKUP,
    RECOVERY_STATUS_RESTORING,
    RECOVERY_STATUS_VERIFYING,
    RECOVERY_STATUS_COMPLETED,
    RECOVERY_STATUS_FAILED,
    RECOVERY_STATUS_CANCELLED,
    RECOVERY_STATUS_MAX
} recovery_status_t;

/* Recovery Progress */
typedef struct {
    recovery_status_t status;
    uint64_t total_bytes;         /* Total bytes to process */
    uint64_t processed_bytes;     /* Bytes processed so far */
    uint32_t percentage;          /* Progress percentage */
    uint64_t bytes_per_second;    /* Transfer rate */
    time_t estimated_completion;  /* Estimated completion time */
    char current_operation[256];  /* Current operation description */
    char current_file[512];       /* Current file being processed */
} recovery_progress_t;

/* Emergency Boot Environment */
typedef struct {
    bool available;               /* Emergency boot environment available */
    char boot_partition[64];      /* Emergency boot partition */
    char kernel_path[512];        /* Emergency kernel path */
    char initrd_path[512];        /* Emergency initrd path */
    char recovery_tools[16][256]; /* Available recovery tools */
    uint32_t tool_count;         /* Number of recovery tools */
} emergency_boot_t;

/* System Health Check */
typedef struct {
    bool filesystem_check;        /* Filesystem integrity check */
    bool boot_loader_check;       /* Boot loader integrity */
    bool kernel_check;           /* Kernel integrity */
    bool driver_check;           /* Driver compatibility check */
    bool configuration_check;     /* Configuration validity */
    bool dependency_check;        /* Dependency resolution check */
    
    /* Health scores (0-100) */
    uint32_t overall_health;     /* Overall system health */
    uint32_t boot_health;        /* Boot system health */
    uint32_t filesystem_health;  /* Filesystem health */
    uint32_t application_health; /* Application health */
    
    /* Issues found */
    uint32_t critical_issues;    /* Number of critical issues */
    uint32_t warning_issues;     /* Number of warning issues */
    char issues[32][256];        /* Issue descriptions */
} system_health_t;

/* Recovery System State */
typedef struct {
    bool initialized;
    pthread_mutex_t lock;
    
    /* Configuration */
    backup_config_t config;
    
    /* Recovery points */
    recovery_point_t recovery_points[256];
    uint32_t recovery_point_count;
    char current_recovery_point[64]; /* Currently active recovery point */
    
    /* Progress tracking */
    recovery_progress_t progress;
    
    /* Emergency boot */
    emergency_boot_t emergency_boot;
    
    /* System health */
    system_health_t health;
    
    /* Statistics */
    struct {
        uint64_t total_backups_created;
        uint64_t total_restores_performed;
        uint64_t total_bytes_backed_up;
        uint64_t total_backup_time_seconds;
        uint64_t successful_recoveries;
        uint64_t failed_recoveries;
        time_t last_backup_time;
        time_t last_verification_time;
    } stats;
    
    /* Threading */
    pthread_t backup_thread;
    pthread_t verification_thread;
    pthread_t cleanup_thread;
    bool threads_running;
} recovery_system_t;

/* Restore Options */
typedef struct {
    recovery_mode_t mode;         /* Recovery mode */
    bool preserve_user_data;      /* Preserve user data during restore */
    bool restore_configuration;   /* Restore system configuration */
    bool restore_applications;    /* Restore applications */
    bool restore_boot_loader;     /* Restore boot loader */
    bool verify_after_restore;    /* Verify system after restore */
    
    /* Selective restore */
    bool selective_restore;       /* Enable selective file restore */
    uint32_t include_path_count;
    char include_paths[64][512];  /* Paths to include in restore */
    uint32_t exclude_path_count;
    char exclude_paths[64][512];  /* Paths to exclude from restore */
    
    /* Advanced options */
    bool force_restore;          /* Force restore even if newer files exist */
    bool create_restore_point;   /* Create recovery point before restore */
    char target_location[512];   /* Alternative restore location */
} restore_options_t;

/* Function Prototypes */

/* System Initialization */
int recovery_system_init(void);
int recovery_system_cleanup(void);
int recovery_load_configuration(const char *config_file);
int recovery_save_configuration(const char *config_file);

/* Recovery Point Management */
int recovery_create_point(const char *name, const char *description, 
                         recovery_type_t type, backup_type_t backup_type);
int recovery_delete_point(const char *recovery_point_id);
int recovery_list_points(recovery_point_t *points, uint32_t max_count, uint32_t *count);
recovery_point_t *recovery_find_point(const char *recovery_point_id);
int recovery_verify_point(const char *recovery_point_id);

/* Backup Operations */
int backup_create_full(const char *name, const char *description);
int backup_create_incremental(const char *parent_id, const char *name);
int backup_create_snapshot(const char *name);
int backup_schedule_automatic(void);
int backup_cancel_current(void);

/* Restore Operations */
int recovery_restore_system(const char *recovery_point_id, const restore_options_t *options);
int recovery_restore_files(const char *recovery_point_id, const char **file_paths, 
                          uint32_t file_count, const char *target_dir);
int recovery_restore_boot_environment(const char *recovery_point_id);
int recovery_restore_configuration(const char *recovery_point_id);

/* Emergency Recovery */
int recovery_enter_emergency_mode(void);
int recovery_exit_emergency_mode(void);
int recovery_boot_from_emergency(void);
int recovery_repair_boot_loader(void);
int recovery_repair_filesystem(const char *device_path);
int recovery_reset_to_factory(void);

/* System Health */
int recovery_check_system_health(system_health_t *health);
int recovery_run_diagnostics(void);
int recovery_fix_issues(bool auto_fix);
int recovery_validate_recovery_point(const char *recovery_point_id);

/* Rollback System */
int recovery_enable_rollback(void);
int recovery_create_rollback_point(const char *operation_name);
int recovery_rollback_last_operation(void);
int recovery_rollback_to_point(const char *recovery_point_id);
int recovery_list_rollback_points(recovery_point_t *points, uint32_t max_count, uint32_t *count);

/* Progress Monitoring */
recovery_progress_t *recovery_get_progress(void);
int recovery_set_progress_callback(void (*callback)(const recovery_progress_t *progress));

/* Configuration Management */
int recovery_set_backup_schedule(uint32_t interval_hours, uint32_t backup_time);
int recovery_set_retention_policy(uint32_t retention_days, uint32_t max_points);
int recovery_add_include_pattern(const char *pattern);
int recovery_add_exclude_pattern(const char *pattern);
int recovery_set_compression(compression_type_t compression);
int recovery_set_encryption(encryption_type_t encryption, const char *key);

/* Storage Management */
int recovery_add_storage_location(backup_storage_t type, const char *location);
int recovery_test_storage_location(backup_storage_t type, const char *location);
int recovery_cleanup_old_backups(void);
int recovery_optimize_storage(void);

/* Import/Export */
int recovery_export_point(const char *recovery_point_id, const char *export_path);
int recovery_import_point(const char *import_path);
int recovery_clone_system(const char *target_device);

/* Security */
int recovery_verify_integrity(const char *recovery_point_id);
int recovery_audit_changes(const char *recovery_point_id);
int recovery_secure_delete(const char *recovery_point_id);

/* Network Operations */
int recovery_setup_network_backup(const char *server_url, const char *credentials);
int recovery_sync_to_cloud(const char *recovery_point_id);
int recovery_download_from_cloud(const char *recovery_point_id);

/* Utilities */
const char *recovery_type_name(recovery_type_t type);
const char *backup_type_name(backup_type_t type);
const char *recovery_mode_name(recovery_mode_t mode);
const char *recovery_status_name(recovery_status_t status);
uint64_t recovery_calculate_space_required(recovery_type_t type);
bool recovery_is_space_available(uint64_t required_bytes);
time_t recovery_estimate_backup_time(recovery_type_t type);

#endif /* LIMITLESSOS_SYSTEM_RECOVERY_H */