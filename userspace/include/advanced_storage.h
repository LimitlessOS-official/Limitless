/*
 * advanced_storage.h - LimitlessOS Advanced Storage Management Header
 * 
 * Comprehensive header for advanced storage features including GPT,
 * LVM, RAID, encryption, and performance monitoring.
 */

#ifndef LIMITLESS_ADVANCED_STORAGE_H
#define LIMITLESS_ADVANCED_STORAGE_H

#include <stdint.h>
#include <stdbool.h>
#include <uuid/uuid.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Version information */
#define LIMITLESS_STORAGE_VERSION_MAJOR    1
#define LIMITLESS_STORAGE_VERSION_MINOR    0
#define LIMITLESS_STORAGE_VERSION_PATCH    0

/* Maximum limits */
#define MAX_STORAGE_DEVICES               64
#define MAX_GPT_PARTITIONS               128
#define MAX_LVM_PHYSICAL_VOLUMES          64
#define MAX_LVM_VOLUME_GROUPS             16
#define MAX_LVM_LOGICAL_VOLUMES           64
#define MAX_RAID_ARRAYS                   16
#define MAX_ENCRYPTED_DEVICES             32
#define MAX_DEVICE_NAME                  256
#define MAX_FILESYSTEM_TYPES              32

/* GPT Management */
typedef struct gpt_partition_info {
    uint32_t number;                     /* Partition number */
    uuid_t type_guid;                    /* Partition type GUID */
    uuid_t unique_guid;                  /* Unique partition GUID */
    uint64_t start_lba;                  /* Starting LBA */
    uint64_t end_lba;                    /* Ending LBA */
    uint64_t size_bytes;                 /* Size in bytes */
    uint64_t attributes;                 /* Partition attributes */
    char name[72];                       /* Partition name (UTF-8) */
    char type_name[64];                  /* Human-readable type name */
} gpt_partition_info_t;

typedef struct gpt_disk_info {
    char device_path[MAX_DEVICE_NAME];   /* Device path */
    uint64_t disk_size;                  /* Disk size in bytes */
    uuid_t disk_guid;                    /* Disk GUID */
    uint32_t sector_size;                /* Logical sector size */
    uint32_t partition_count;            /* Number of partitions */
    gpt_partition_info_t partitions[MAX_GPT_PARTITIONS];
} gpt_disk_info_t;

/* GPT API functions */
int gpt_init(void);
int gpt_create_table(const char* device);
int gpt_add_partition(const char* device, const char* type, uint64_t start, uint64_t size, const char* name);
int gpt_delete_partition(const char* device, uint32_t partition_number);
int gpt_resize_partition(const char* device, uint32_t partition_number, uint64_t new_size);
int gpt_list_partitions(const char* device, gpt_partition_info_t* partitions, uint32_t* count);
int gpt_get_disk_info(const char* device, gpt_disk_info_t* info);
int gpt_set_partition_name(const char* device, uint32_t partition_number, const char* name);
int gpt_set_partition_attributes(const char* device, uint32_t partition_number, uint64_t attributes);

/* LVM Management */
typedef enum {
    LVM_PV_ACTIVE,
    LVM_PV_INACTIVE,
    LVM_PV_MISSING,
    LVM_PV_UNKNOWN
} lvm_pv_status_t;

typedef enum {
    LVM_VG_ACTIVE,
    LVM_VG_INACTIVE,
    LVM_VG_PARTIAL,
    LVM_VG_UNKNOWN
} lvm_vg_status_t;

typedef enum {
    LVM_LV_ACTIVE,
    LVM_LV_INACTIVE,
    LVM_LV_SUSPENDED,
    LVM_LV_UNKNOWN
} lvm_lv_status_t;

typedef struct lvm_pv_info {
    char device_path[MAX_DEVICE_NAME];   /* PV device path */
    uuid_t pv_uuid;                      /* PV UUID */
    char vg_name[64];                    /* Volume group name */
    uint64_t size;                       /* PV size in bytes */
    uint64_t free_size;                  /* Free space in bytes */
    uint64_t pe_size;                    /* Physical extent size */
    uint64_t pe_count;                   /* Total PE count */
    uint64_t pe_free;                    /* Free PE count */
    lvm_pv_status_t status;              /* PV status */
} lvm_pv_info_t;

typedef struct lvm_vg_info {
    char name[64];                       /* VG name */
    uuid_t vg_uuid;                      /* VG UUID */
    uint64_t size;                       /* Total VG size */
    uint64_t free_size;                  /* Free space */
    uint64_t pe_size;                    /* Physical extent size */
    uint64_t pe_count;                   /* Total PE count */
    uint64_t pe_free;                    /* Free PE count */
    uint32_t pv_count;                   /* Number of PVs */
    uint32_t lv_count;                   /* Number of LVs */
    lvm_vg_status_t status;              /* VG status */
} lvm_vg_info_t;

typedef struct lvm_lv_info {
    char name[64];                       /* LV name */
    char vg_name[64];                    /* Parent VG name */
    char full_name[128];                 /* Full name (vg/lv) */
    char device_path[MAX_DEVICE_NAME];   /* Device path */
    uuid_t lv_uuid;                      /* LV UUID */
    uint64_t size;                       /* LV size in bytes */
    uint64_t le_count;                   /* Logical extent count */
    lvm_lv_status_t status;              /* LV status */
    bool encrypted;                      /* Is encrypted */
} lvm_lv_info_t;

/* LVM API functions */
int lvm_init(void);
int lvm_scan_devices(void);
int lvm_create_pv(const char* device);
int lvm_remove_pv(const char* device);
int lvm_create_vg(const char* vg_name, const char** pv_devices, uint32_t pv_count);
int lvm_extend_vg(const char* vg_name, const char** pv_devices, uint32_t pv_count);
int lvm_remove_vg(const char* vg_name);
int lvm_create_lv(const char* vg_name, const char* lv_name, uint64_t size);
int lvm_extend_lv(const char* vg_name, const char* lv_name, uint64_t additional_size);
int lvm_remove_lv(const char* vg_name, const char* lv_name);
int lvm_activate_lv(const char* vg_name, const char* lv_name);
int lvm_deactivate_lv(const char* vg_name, const char* lv_name);
int lvm_list_pvs(lvm_pv_info_t* pvs, uint32_t* count);
int lvm_list_vgs(lvm_vg_info_t* vgs, uint32_t* count);
int lvm_list_lvs(lvm_lv_info_t* lvs, uint32_t* count);
int lvm_get_pv_info(const char* device, lvm_pv_info_t* info);
int lvm_get_vg_info(const char* vg_name, lvm_vg_info_t* info);
int lvm_get_lv_info(const char* vg_name, const char* lv_name, lvm_lv_info_t* info);

/* RAID Management */
typedef enum {
    RAID_LEVEL_0,                        /* Striping */
    RAID_LEVEL_1,                        /* Mirroring */
    RAID_LEVEL_4,                        /* Dedicated parity */
    RAID_LEVEL_5,                        /* Distributed parity */
    RAID_LEVEL_6,                        /* Double parity */
    RAID_LEVEL_10,                       /* Stripe of mirrors */
    RAID_LEVEL_LINEAR                    /* Concatenation */
} raid_level_t;

typedef enum {
    RAID_STATE_CLEAN,
    RAID_STATE_ACTIVE,
    RAID_STATE_DEGRADED,
    RAID_STATE_RECOVERING,
    RAID_STATE_RESHAPING,
    RAID_STATE_INACTIVE,
    RAID_STATE_UNKNOWN
} raid_state_t;

typedef struct raid_device_info {
    char device_path[MAX_DEVICE_NAME];   /* Device path */
    uint32_t slot;                       /* Device slot number */
    bool active;                         /* Device is active */
    bool failed;                         /* Device has failed */
    bool spare;                          /* Device is spare */
    uint64_t size;                       /* Device size */
} raid_device_info_t;

typedef struct raid_array_info {
    char name[64];                       /* Array name */
    char device_path[MAX_DEVICE_NAME];   /* Array device path */
    raid_level_t level;                  /* RAID level */
    raid_state_t state;                  /* Array state */
    uint64_t array_size;                 /* Total array size */
    uint64_t chunk_size;                 /* Chunk size */
    uint32_t device_count;               /* Total devices */
    uint32_t active_devices;             /* Active devices */
    uint32_t spare_devices;              /* Spare devices */
    uint32_t failed_devices;             /* Failed devices */
    uint32_t rebuild_progress;           /* Rebuild progress (0-100) */
    raid_device_info_t devices[16];      /* Member devices */
} raid_array_info_t;

/* RAID API functions */
int raid_init(void);
int raid_create_array(const char* name, raid_level_t level, const char** devices, 
                     uint32_t device_count, uint64_t chunk_size);
int raid_destroy_array(const char* name);
int raid_add_device(const char* array_name, const char* device);
int raid_remove_device(const char* array_name, const char* device);
int raid_mark_failed(const char* array_name, const char* device);
int raid_start_array(const char* name);
int raid_stop_array(const char* name);
int raid_list_arrays(raid_array_info_t* arrays, uint32_t* count);
int raid_get_array_info(const char* name, raid_array_info_t* info);
int raid_reshape_array(const char* name, uint32_t new_device_count);
int raid_check_array(const char* name);
int raid_repair_array(const char* name);

/* Encryption Management (LUKS) */
typedef enum {
    LUKS_CIPHER_AES128_CBC,
    LUKS_CIPHER_AES192_CBC,
    LUKS_CIPHER_AES256_CBC,
    LUKS_CIPHER_AES128_XTS,
    LUKS_CIPHER_AES256_XTS,
    LUKS_CIPHER_SERPENT256_CBC,
    LUKS_CIPHER_TWOFISH256_CBC
} luks_cipher_type_t;

typedef enum {
    LUKS_HASH_SHA1,
    LUKS_HASH_SHA256,
    LUKS_HASH_SHA512,
    LUKS_HASH_RIPEMD160
} luks_hash_type_t;

typedef struct luks_keyslot_info {
    uint32_t slot_number;                /* Key slot number */
    bool active;                         /* Slot is active */
    uint32_t iterations;                 /* PBKDF2 iterations */
    uint32_t key_material_offset;        /* Offset to key material */
} luks_keyslot_info_t;

typedef struct luks_device_info {
    char device_path[MAX_DEVICE_NAME];   /* Source device path */
    char mapped_name[64];                /* Mapped device name */
    char mapped_path[MAX_DEVICE_NAME];   /* Mapped device path */
    char uuid[40];                       /* LUKS UUID */
    luks_cipher_type_t cipher;           /* Cipher type */
    luks_hash_type_t hash;               /* Hash type */
    uint32_t key_size;                   /* Key size in bytes */
    uint64_t device_size;                /* Device size */
    uint32_t active_keyslots;            /* Number of active keyslots */
    luks_keyslot_info_t keyslots[8];     /* Keyslot information */
} luks_device_info_t;

/* Encryption API functions */
int luks_init(void);
int luks_format(const char* device, const char* cipher, const char* hash, 
               const char* passphrase, uint32_t key_size);
int luks_open_device(const char* device, const char* name, const char* passphrase);
int luks_close_device(const char* name);
int luks_add_key(const char* device, const char* existing_passphrase, const char* new_passphrase);
int luks_remove_key(const char* device, const char* passphrase);
int luks_change_key(const char* device, const char* old_passphrase, const char* new_passphrase);
int luks_list_devices(luks_device_info_t* devices, uint32_t* count);
int luks_get_device_info(const char* device, luks_device_info_t* info);
int luks_backup_header(const char* device, const char* backup_file);
int luks_restore_header(const char* device, const char* backup_file);

/* Filesystem Management */
typedef enum {
    FS_TYPE_EXT2,
    FS_TYPE_EXT3,
    FS_TYPE_EXT4,
    FS_TYPE_XFS,
    FS_TYPE_BTRFS,
    FS_TYPE_F2FS,
    FS_TYPE_NTFS,
    FS_TYPE_FAT32,
    FS_TYPE_EXFAT,
    FS_TYPE_ZFS,
    FS_TYPE_UNKNOWN
} filesystem_type_t;

typedef struct filesystem_info {
    char device_path[MAX_DEVICE_NAME];   /* Device path */
    char mount_point[MAX_DEVICE_NAME];   /* Mount point */
    filesystem_type_t type;              /* Filesystem type */
    char label[64];                      /* Filesystem label */
    char uuid[40];                       /* Filesystem UUID */
    uint64_t total_size;                 /* Total size in bytes */
    uint64_t used_size;                  /* Used size in bytes */
    uint64_t available_size;             /* Available size in bytes */
    uint64_t total_inodes;               /* Total inodes */
    uint64_t used_inodes;                /* Used inodes */
    bool mounted;                        /* Is mounted */
    bool read_only;                      /* Is read-only */
} filesystem_info_t;

/* Filesystem API functions */
int fs_init(void);
int fs_create(const char* device, filesystem_type_t type, const char* label);
int fs_mount(const char* device, const char* mount_point, const char* options);
int fs_unmount(const char* mount_point);
int fs_check(const char* device, bool repair);
int fs_resize(const char* device, uint64_t new_size);
int fs_set_label(const char* device, const char* label);
int fs_list_filesystems(filesystem_info_t* filesystems, uint32_t* count);
int fs_get_info(const char* device, filesystem_info_t* info);
const char* fs_type_to_string(filesystem_type_t type);
filesystem_type_t fs_detect_type(const char* device);

/* Performance Monitoring */
typedef struct storage_stats {
    char device_name[64];                /* Device name */
    uint64_t read_ops;                   /* Read operations */
    uint64_t write_ops;                  /* Write operations */
    uint64_t read_bytes;                 /* Bytes read */
    uint64_t write_bytes;                /* Bytes written */
    uint64_t read_time_ms;               /* Read time in milliseconds */
    uint64_t write_time_ms;              /* Write time in milliseconds */
    uint64_t io_time_ms;                 /* Total I/O time */
    uint64_t weighted_io_time_ms;        /* Weighted I/O time */
    uint32_t queue_depth;                /* Current queue depth */
    double utilization;                  /* Device utilization (0.0-1.0) */
} storage_stats_t;

typedef struct performance_metrics {
    double read_iops;                    /* Read IOPS */
    double write_iops;                   /* Write IOPS */
    double read_throughput_mbps;         /* Read throughput (MB/s) */
    double write_throughput_mbps;        /* Write throughput (MB/s) */
    double avg_read_latency_ms;          /* Average read latency */
    double avg_write_latency_ms;         /* Average write latency */
    double queue_depth;                  /* Average queue depth */
    double utilization;                  /* Device utilization */
} performance_metrics_t;

/* Performance monitoring API functions */
int perf_init(void);
int perf_start_monitoring(const char* device);
int perf_stop_monitoring(const char* device);
int perf_get_stats(const char* device, storage_stats_t* stats);
int perf_get_metrics(const char* device, performance_metrics_t* metrics);
int perf_reset_counters(const char* device);
int perf_list_monitored_devices(char devices[][64], uint32_t* count);

/* Benchmarking */
typedef enum {
    BENCH_SEQUENTIAL_READ,
    BENCH_SEQUENTIAL_WRITE,
    BENCH_RANDOM_READ,
    BENCH_RANDOM_WRITE,
    BENCH_MIXED_WORKLOAD,
    BENCH_LATENCY_TEST
} benchmark_type_t;

typedef struct benchmark_config {
    benchmark_type_t type;               /* Benchmark type */
    uint64_t test_size;                  /* Test size in bytes */
    uint32_t block_size;                 /* Block size */
    uint32_t thread_count;               /* Number of threads */
    uint32_t queue_depth;                /* Queue depth */
    uint32_t duration;                   /* Test duration in seconds */
    uint32_t read_percentage;            /* Read percentage for mixed workload */
    bool direct_io;                      /* Use direct I/O */
    bool verify_data;                    /* Verify written data */
} benchmark_config_t;

typedef struct benchmark_results {
    double read_iops;                    /* Read IOPS */
    double write_iops;                   /* Write IOPS */
    double read_mbps;                    /* Read throughput */
    double write_mbps;                   /* Write throughput */
    double avg_latency_us;               /* Average latency (microseconds) */
    double min_latency_us;               /* Minimum latency */
    double max_latency_us;               /* Maximum latency */
    double p50_latency_us;               /* 50th percentile latency */
    double p95_latency_us;               /* 95th percentile latency */
    double p99_latency_us;               /* 99th percentile latency */
    uint64_t total_operations;           /* Total operations */
    uint64_t total_bytes;                /* Total bytes transferred */
    uint64_t error_count;                /* Number of errors */
    double test_duration;                /* Actual test duration */
} benchmark_results_t;

/* Benchmarking API functions */
int benchmark_init(const char* device);
int benchmark_run(const char* device, const benchmark_config_t* config, benchmark_results_t* results);
int benchmark_comprehensive(const char* device, benchmark_results_t results[], uint32_t* result_count);

/* Storage Manager */
typedef struct storage_manager_info {
    uint32_t total_devices;              /* Total storage devices */
    uint32_t gpt_disks;                  /* GPT partitioned disks */
    uint32_t lvm_pvs;                    /* LVM physical volumes */
    uint32_t lvm_vgs;                    /* LVM volume groups */
    uint32_t lvm_lvs;                    /* LVM logical volumes */
    uint32_t raid_arrays;                /* RAID arrays */
    uint32_t encrypted_devices;          /* Encrypted devices */
    uint32_t mounted_filesystems;        /* Mounted filesystems */
    uint64_t total_storage;              /* Total storage capacity */
    uint64_t used_storage;               /* Used storage space */
    uint64_t available_storage;          /* Available storage space */
} storage_manager_info_t;

/* Storage Manager API functions */
int storage_manager_init(void);
int storage_manager_shutdown(void);
int storage_manager_scan_devices(void);
int storage_manager_get_info(storage_manager_info_t* info);
int storage_manager_list_devices(char devices[][MAX_DEVICE_NAME], uint32_t* count);
int storage_manager_get_device_type(const char* device, char* type, size_t type_size);
int storage_manager_is_device_busy(const char* device, bool* busy);

/* Error codes */
#define STORAGE_SUCCESS                  0
#define STORAGE_ERROR_INVALID_PARAM     -1
#define STORAGE_ERROR_DEVICE_NOT_FOUND  -2
#define STORAGE_ERROR_PERMISSION_DENIED -3
#define STORAGE_ERROR_DEVICE_BUSY       -4
#define STORAGE_ERROR_INSUFFICIENT_SPACE -5
#define STORAGE_ERROR_NOT_SUPPORTED     -6
#define STORAGE_ERROR_IO_ERROR          -7
#define STORAGE_ERROR_CORRUPTION        -8
#define STORAGE_ERROR_TIMEOUT           -9
#define STORAGE_ERROR_OUT_OF_MEMORY     -10
#define STORAGE_ERROR_ALREADY_EXISTS    -11
#define STORAGE_ERROR_NOT_INITIALIZED   -12

/* Utility functions */
const char* storage_error_string(int error_code);
uint64_t storage_size_from_string(const char* size_str);  /* Parse size strings like "1GB", "500MB" */
void storage_size_to_string(uint64_t size, char* buffer, size_t buffer_size);
bool storage_is_block_device(const char* path);
bool storage_device_exists(const char* path);
int storage_get_device_size(const char* device, uint64_t* size);
int storage_get_sector_size(const char* device, uint32_t* sector_size);

#ifdef __cplusplus
}
#endif

#endif /* LIMITLESS_ADVANCED_STORAGE_H */