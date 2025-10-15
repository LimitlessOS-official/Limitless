#ifndef LIMITLESS_BACKUP_H
#define LIMITLESS_BACKUP_H

#include "types.h"
#include "vfs.h"

/* Maximum backup configurations */
#define BACKUP_MAX_CONFIGS 64
#define BACKUP_MAX_PATH_LEN 512
#define BACKUP_MAX_NAME_LEN 128
#define BACKUP_MAX_EXCLUDE_PATTERNS 32
#define BACKUP_MAX_SNAPSHOTS 256

/* Backup types */
typedef enum {
    BACKUP_TYPE_FULL = 0,        /* Complete backup of all data */
    BACKUP_TYPE_INCREMENTAL = 1, /* Only changed files since last backup */
    BACKUP_TYPE_DIFFERENTIAL = 2, /* Changed files since last full backup */
    BACKUP_TYPE_SNAPSHOT = 3     /* Point-in-time filesystem snapshot */
} backup_type_t;

/* Backup compression methods */
typedef enum {
    BACKUP_COMPRESS_NONE = 0,
    BACKUP_COMPRESS_GZIP = 1,
    BACKUP_COMPRESS_BZIP2 = 2,
    BACKUP_COMPRESS_LZMA = 3,
    BACKUP_COMPRESS_LZ4 = 4
} backup_compression_t;

/* Backup encryption methods */
typedef enum {
    BACKUP_ENCRYPT_NONE = 0,
    BACKUP_ENCRYPT_AES256 = 1,
    BACKUP_ENCRYPT_CHACHA20 = 2
} backup_encryption_t;

/* Backup schedule types */
typedef enum {
    BACKUP_SCHEDULE_MANUAL = 0,
    BACKUP_SCHEDULE_HOURLY = 1,
    BACKUP_SCHEDULE_DAILY = 2,
    BACKUP_SCHEDULE_WEEKLY = 3,
    BACKUP_SCHEDULE_MONTHLY = 4,
    BACKUP_SCHEDULE_CUSTOM = 5
} backup_schedule_t;

/* Backup status */
typedef enum {
    BACKUP_STATUS_IDLE = 0,
    BACKUP_STATUS_RUNNING = 1,
    BACKUP_STATUS_COMPLETED = 2,
    BACKUP_STATUS_FAILED = 3,
    BACKUP_STATUS_CANCELLED = 4,
    BACKUP_STATUS_VERIFYING = 5
} backup_status_t;

/* File metadata for backup */
typedef struct {
    char path[BACKUP_MAX_PATH_LEN];
    uint64_t size;
    uint64_t mtime;              /* Last modification time */
    uint64_t ctime;              /* Creation time */
    uint32_t mode;               /* File permissions */
    uint32_t uid;                /* Owner user ID */
    uint32_t gid;                /* Owner group ID */
    uint32_t checksum;           /* File content checksum */
    bool is_directory;
    bool is_symlink;
    char symlink_target[BACKUP_MAX_PATH_LEN];
} backup_file_metadata_t;

/* Backup configuration */
typedef struct {
    uint32_t config_id;
    char name[BACKUP_MAX_NAME_LEN];
    bool enabled;
    
    /* Source and destination */
    char source_path[BACKUP_MAX_PATH_LEN];
    char destination_path[BACKUP_MAX_PATH_LEN];
    
    /* Backup settings */
    backup_type_t type;
    backup_compression_t compression;
    backup_encryption_t encryption;
    char encryption_key[64];     /* Encryption key (base64 encoded) */
    
    /* Scheduling */
    backup_schedule_t schedule;
    uint64_t next_backup_time;   /* Next scheduled backup timestamp */
    uint32_t retention_days;     /* Days to keep backups */
    uint32_t max_snapshots;      /* Maximum snapshots to keep */
    
    /* Filters */
    char exclude_patterns[BACKUP_MAX_EXCLUDE_PATTERNS][BACKUP_MAX_PATH_LEN];
    uint8_t exclude_count;
    bool follow_symlinks;
    bool backup_hidden_files;
    uint64_t max_file_size;      /* Skip files larger than this */
    
    /* Statistics */
    uint64_t last_backup_time;
    uint64_t last_backup_size;
    uint32_t total_backups;
    uint32_t failed_backups;
    
    /* Runtime state */
    backup_status_t status;
    uint64_t current_progress;   /* Bytes processed */
    uint64_t total_size;         /* Total bytes to process */
    char current_file[BACKUP_MAX_PATH_LEN];
    uint32_t error_code;
} backup_config_t;

/* Backup snapshot information */
typedef struct {
    uint32_t snapshot_id;
    uint32_t config_id;
    char name[BACKUP_MAX_NAME_LEN];
    backup_type_t type;
    uint64_t created_time;
    uint64_t backup_size;        /* Size of backup data */
    uint64_t original_size;      /* Size of original data */
    uint32_t file_count;         /* Number of files in backup */
    uint32_t directory_count;    /* Number of directories */
    char backup_path[BACKUP_MAX_PATH_LEN];
    char metadata_path[BACKUP_MAX_PATH_LEN];
    bool verified;               /* Backup integrity verified */
    uint32_t checksum;           /* Backup integrity checksum */
} backup_snapshot_t;

/* Backup restore request */
typedef struct {
    uint32_t snapshot_id;
    char source_path[BACKUP_MAX_PATH_LEN];   /* Path in backup */
    char destination_path[BACKUP_MAX_PATH_LEN]; /* Restore destination */
    bool overwrite_existing;
    bool preserve_permissions;
    bool verify_checksums;
} backup_restore_request_t;

/* Backup progress callback */
typedef void (*backup_progress_callback_t)(uint32_t config_id, uint64_t processed, 
                                          uint64_t total, const char* current_file);

/* Backup verification result */
typedef struct {
    uint32_t snapshot_id;
    bool valid;
    uint32_t files_checked;
    uint32_t files_corrupted;
    uint64_t bytes_verified;
    char error_message[256];
} backup_verification_result_t;

/* Backup statistics */
typedef struct {
    uint32_t active_configs;
    uint32_t total_snapshots;
    uint64_t total_backup_size;
    uint32_t backups_running;
    uint32_t backups_completed_today;
    uint32_t backups_failed_today;
    uint64_t bytes_backed_up_today;
    uint64_t average_backup_time;
    float compression_ratio;
} backup_statistics_t;

/* Function declarations */
status_t backup_init(void);
status_t backup_shutdown(void);

/* Configuration management */
status_t backup_create_config(const backup_config_t* config, uint32_t* out_config_id);
status_t backup_update_config(uint32_t config_id, const backup_config_t* config);
status_t backup_delete_config(uint32_t config_id);
status_t backup_get_config(uint32_t config_id, backup_config_t* out_config);
status_t backup_list_configs(uint32_t* out_config_ids, uint32_t* in_out_count);

/* Backup operations */
status_t backup_start(uint32_t config_id);
status_t backup_start_async(uint32_t config_id, backup_progress_callback_t callback);
status_t backup_cancel(uint32_t config_id);
status_t backup_get_status(uint32_t config_id, backup_status_t* out_status);
status_t backup_get_progress(uint32_t config_id, uint64_t* out_processed, uint64_t* out_total);

/* Snapshot management */
status_t backup_list_snapshots(uint32_t config_id, backup_snapshot_t* out_snapshots, uint32_t* in_out_count);
status_t backup_get_snapshot(uint32_t snapshot_id, backup_snapshot_t* out_snapshot);
status_t backup_delete_snapshot(uint32_t snapshot_id);
status_t backup_verify_snapshot(uint32_t snapshot_id, backup_verification_result_t* out_result);

/* Restore operations */
status_t backup_restore_file(const backup_restore_request_t* request);
status_t backup_restore_directory(const backup_restore_request_t* request);
status_t backup_list_backup_contents(uint32_t snapshot_id, const char* path,
                                   backup_file_metadata_t* out_files, uint32_t* in_out_count);

/* Scheduling */
status_t backup_scheduler_start(void);
status_t backup_scheduler_stop(void);
status_t backup_check_scheduled_backups(void);
status_t backup_set_schedule(uint32_t config_id, backup_schedule_t schedule, uint64_t custom_interval);

/* Retention management */
status_t backup_cleanup_old_snapshots(uint32_t config_id);
status_t backup_set_retention_policy(uint32_t config_id, uint32_t retention_days, uint32_t max_snapshots);

/* Compression and encryption */
status_t backup_compress_data(const uint8_t* input, size_t input_size, 
                             backup_compression_t method, uint8_t* output, size_t* output_size);
status_t backup_decompress_data(const uint8_t* input, size_t input_size,
                               backup_compression_t method, uint8_t* output, size_t* output_size);
status_t backup_encrypt_data(const uint8_t* input, size_t input_size,
                            backup_encryption_t method, const char* key,
                            uint8_t* output, size_t* output_size);
status_t backup_decrypt_data(const uint8_t* input, size_t input_size,
                            backup_encryption_t method, const char* key,
                            uint8_t* output, size_t* output_size);

/* Utilities */
status_t backup_calculate_file_checksum(const char* file_path, uint32_t* out_checksum);
status_t backup_get_directory_size(const char* path, uint64_t* out_size, uint32_t* out_file_count);
status_t backup_create_metadata_file(uint32_t snapshot_id, const backup_file_metadata_t* files, uint32_t file_count);
status_t backup_load_metadata_file(uint32_t snapshot_id, backup_file_metadata_t* out_files, uint32_t* in_out_count);

/* Statistics and monitoring */
status_t backup_get_statistics(backup_statistics_t* out_stats);
status_t backup_reset_statistics(void);
status_t backup_export_configuration(uint32_t config_id, const char* export_path);
status_t backup_import_configuration(const char* import_path, uint32_t* out_config_id);

/* Events and notifications */
typedef void (*backup_event_callback_t)(uint32_t config_id, backup_status_t status, const char* message);
status_t backup_register_event_callback(backup_event_callback_t callback);
status_t backup_unregister_event_callback(backup_event_callback_t callback);

/* Error codes */
#define BACKUP_ERROR_INSUFFICIENT_SPACE  0x1001
#define BACKUP_ERROR_SOURCE_NOT_FOUND    0x1002
#define BACKUP_ERROR_DESTINATION_INVALID 0x1003
#define BACKUP_ERROR_COMPRESSION_FAILED  0x1004
#define BACKUP_ERROR_ENCRYPTION_FAILED   0x1005
#define BACKUP_ERROR_CHECKSUM_MISMATCH   0x1006
#define BACKUP_ERROR_PERMISSION_DENIED   0x1007
#define BACKUP_ERROR_BACKUP_CORRUPTED    0x1008

#endif /* LIMITLESS_BACKUP_H */