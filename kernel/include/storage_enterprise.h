/**
 * LimitlessOS Universal Storage Framework
 * Enterprise-grade filesystem and storage subsystem
 */

#ifndef LIMITLESS_STORAGE_ENTERPRISE_H
#define LIMITLESS_STORAGE_ENTERPRISE_H

#include <stdint.h>
#include <stdbool.h>
#include "kernel.h"

/* Storage Framework Version */
#define LIMITLESS_STORAGE_VERSION_MAJOR 2
#define LIMITLESS_STORAGE_VERSION_MINOR 0

/* Maximum limits */
#define MAX_STORAGE_DEVICES         256
#define MAX_FILESYSTEMS             64
#define MAX_PARTITIONS              1024
#define MAX_MOUNT_POINTS            512
#define MAX_OPEN_FILES              65536
#define MAX_FILE_SYSTEMS            32
#define MAX_ENCRYPTION_KEYS         256
#define MAX_COMPRESSION_CONTEXTS    128

/* Filesystem Types */
typedef enum {
    FS_TYPE_UNKNOWN,
    FS_TYPE_EXT2,
    FS_TYPE_EXT3,
    FS_TYPE_EXT4,
    FS_TYPE_XFS,
    FS_TYPE_BTRFS,
    FS_TYPE_ZFS,
    FS_TYPE_F2FS,
    FS_TYPE_NTFS,
    FS_TYPE_REFS,               /* Windows ReFS */
    FS_TYPE_HFS_PLUS,           /* macOS HFS+ */
    FS_TYPE_APFS,               /* Apple File System */
    FS_TYPE_FAT12,
    FS_TYPE_FAT16,
    FS_TYPE_FAT32,
    FS_TYPE_EXFAT,
    FS_TYPE_UDF,                /* Universal Disk Format */
    FS_TYPE_ISO9660,            /* CD-ROM filesystem */
    FS_TYPE_SQUASHFS,           /* Compressed read-only filesystem */
    FS_TYPE_TMPFS,              /* Temporary filesystem in RAM */
    FS_TYPE_PROCFS,             /* Process filesystem */
    FS_TYPE_SYSFS,              /* System filesystem */
    FS_TYPE_DEVFS,              /* Device filesystem */
    FS_TYPE_NFS,                /* Network File System */
    FS_TYPE_CIFS,               /* Common Internet File System */
    FS_TYPE_FUSE,               /* Filesystem in Userspace */
    FS_TYPE_LIMITLESSFS,        /* LimitlessOS native filesystem */
    FS_TYPE_QUANTUMFS          /* Quantum-encrypted filesystem */
} filesystem_type_t;

/* Storage Device Types */
typedef enum {
    STORAGE_TYPE_UNKNOWN,
    STORAGE_TYPE_HDD,           /* Traditional hard disk */
    STORAGE_TYPE_SSD,           /* SATA/AHCI SSD */
    STORAGE_TYPE_NVME,          /* NVMe SSD */
    STORAGE_TYPE_EMMC,          /* eMMC storage */
    STORAGE_TYPE_SD_CARD,       /* SD/microSD card */
    STORAGE_TYPE_USB_FLASH,     /* USB flash drive */
    STORAGE_TYPE_CD_ROM,        /* CD-ROM drive */
    STORAGE_TYPE_DVD,           /* DVD drive */
    STORAGE_TYPE_BLU_RAY,       /* Blu-ray drive */
    STORAGE_TYPE_TAPE,          /* Tape drive */
    STORAGE_TYPE_FLOPPY,        /* Floppy disk */
    STORAGE_TYPE_RAM_DISK,      /* RAM disk */
    STORAGE_TYPE_NETWORK,       /* Network storage */
    STORAGE_TYPE_VIRTUAL,       /* Virtual disk */
    STORAGE_TYPE_QUANTUM        /* Quantum storage */
} storage_device_type_t;

/* Storage Interface Types */
typedef enum {
    STORAGE_INTERFACE_UNKNOWN,
    STORAGE_INTERFACE_SATA,     /* Serial ATA */
    STORAGE_INTERFACE_PATA,     /* Parallel ATA (IDE) */
    STORAGE_INTERFACE_SCSI,     /* Small Computer System Interface */
    STORAGE_INTERFACE_SAS,      /* Serial Attached SCSI */
    STORAGE_INTERFACE_NVME,     /* NVM Express */
    STORAGE_INTERFACE_USB,      /* Universal Serial Bus */
    STORAGE_INTERFACE_THUNDERBOLT, /* Thunderbolt */
    STORAGE_INTERFACE_ETHERNET, /* Network storage */
    STORAGE_INTERFACE_FC,       /* Fibre Channel */
    STORAGE_INTERFACE_ISCSI,    /* iSCSI */
    STORAGE_INTERFACE_FCP,      /* Fibre Channel Protocol */
    STORAGE_INTERFACE_VIRTUAL   /* Virtual interface */
} storage_interface_t;

/* Encryption Types */
typedef enum {
    ENCRYPTION_NONE,
    ENCRYPTION_AES_128_CBC,
    ENCRYPTION_AES_256_CBC,
    ENCRYPTION_AES_128_XTS,
    ENCRYPTION_AES_256_XTS,
    ENCRYPTION_AES_128_GCM,
    ENCRYPTION_AES_256_GCM,
    ENCRYPTION_CHACHA20_POLY1305,
    ENCRYPTION_SERPENT_256,
    ENCRYPTION_TWOFISH_256,
    ENCRYPTION_POST_QUANTUM,    /* Post-quantum cryptography */
    ENCRYPTION_QUANTUM_KEY      /* Quantum key distribution */
} encryption_type_t;

/* Compression Types */
typedef enum {
    COMPRESSION_NONE,
    COMPRESSION_GZIP,
    COMPRESSION_BZIP2,
    COMPRESSION_LZMA,
    COMPRESSION_LZ4,
    COMPRESSION_LZ4HC,
    COMPRESSION_ZSTD,
    COMPRESSION_SNAPPY,
    COMPRESSION_BROTLI,
    COMPRESSION_XZ,
    COMPRESSION_QUANTUM         /* Quantum compression */
} compression_type_t;

/* RAID Types */
typedef enum {
    RAID_NONE,
    RAID_0,                     /* Striping */
    RAID_1,                     /* Mirroring */
    RAID_5,                     /* Striping with parity */
    RAID_6,                     /* Striping with dual parity */
    RAID_10,                    /* Mirrored stripes */
    RAID_50,                    /* Striped RAID 5 */
    RAID_60,                    /* Striped RAID 6 */
    RAID_Z1,                    /* ZFS RAID-Z */
    RAID_Z2,                    /* ZFS RAID-Z2 */
    RAID_Z3                     /* ZFS RAID-Z3 */
} raid_type_t;

/* File Types */
typedef enum {
    FILE_TYPE_REGULAR,
    FILE_TYPE_DIRECTORY,
    FILE_TYPE_SYMLINK,
    FILE_TYPE_HARDLINK,
    FILE_TYPE_DEVICE_BLOCK,
    FILE_TYPE_DEVICE_CHAR,
    FILE_TYPE_FIFO,
    FILE_TYPE_SOCKET,
    FILE_TYPE_WHITEOUT          /* Union filesystem whiteout */
} file_type_t;

/* File Permissions */
#define FILE_PERM_USER_READ     0400
#define FILE_PERM_USER_WRITE    0200
#define FILE_PERM_USER_EXEC     0100
#define FILE_PERM_GROUP_READ    0040
#define FILE_PERM_GROUP_WRITE   0020
#define FILE_PERM_GROUP_EXEC    0010
#define FILE_PERM_OTHER_READ    0004
#define FILE_PERM_OTHER_WRITE   0002
#define FILE_PERM_OTHER_EXEC    0001
#define FILE_PERM_SETUID        04000
#define FILE_PERM_SETGID        02000
#define FILE_PERM_STICKY        01000

/* File Attributes */
#define FILE_ATTR_READONLY      0x01
#define FILE_ATTR_HIDDEN        0x02
#define FILE_ATTR_SYSTEM        0x04
#define FILE_ATTR_ARCHIVE       0x08
#define FILE_ATTR_COMPRESSED    0x10
#define FILE_ATTR_ENCRYPTED     0x20
#define FILE_ATTR_TEMPORARY     0x40
#define FILE_ATTR_SPARSE        0x80
#define FILE_ATTR_IMMUTABLE     0x100
#define FILE_ATTR_APPEND_ONLY   0x200
#define FILE_ATTR_NO_DUMP       0x400
#define FILE_ATTR_NO_ATIME      0x800

/* Storage Device Information */
typedef struct storage_device {
    uint32_t id;                /* Device ID */
    char name[64];              /* Device name */
    char model[128];            /* Device model */
    char serial[64];            /* Serial number */
    char firmware[32];          /* Firmware version */
    
    storage_device_type_t type; /* Device type */
    storage_interface_t interface; /* Interface type */
    
    /* Capacity */
    uint64_t total_size;        /* Total size in bytes */
    uint64_t usable_size;       /* Usable size in bytes */
    uint32_t sector_size;       /* Sector size in bytes */
    uint64_t total_sectors;     /* Total number of sectors */
    
    /* Performance characteristics */
    uint32_t max_read_speed;    /* MB/s */
    uint32_t max_write_speed;   /* MB/s */
    uint32_t random_read_iops;  /* IOPS */
    uint32_t random_write_iops; /* IOPS */
    uint32_t average_latency;   /* Microseconds */
    
    /* Physical properties */
    bool removable;             /* Removable media */
    bool hot_pluggable;         /* Hot-pluggable */
    bool write_protected;       /* Write protected */
    uint32_t temperature;       /* Temperature in Celsius */
    uint32_t power_consumption; /* Watts */
    
    /* Health and reliability */
    uint32_t health_percentage; /* 0-100% health */
    uint64_t power_on_hours;    /* Total power-on hours */
    uint64_t total_bytes_written; /* Total bytes written */
    uint64_t total_bytes_read;  /* Total bytes read */
    uint32_t error_count;       /* Total error count */
    uint32_t bad_sectors;       /* Number of bad sectors */
    
    /* SMART attributes */
    struct {
        uint8_t id;
        uint16_t flags;
        uint8_t current;
        uint8_t worst;
        uint64_t raw_value;
        uint8_t threshold;
    } smart_attributes[30];
    
    /* Hardware details */
    uint16_t vendor_id;
    uint16_t product_id;
    uint32_t bus_number;
    uint32_t device_number;
    char bus_path[256];
    
    /* Driver information */
    char driver_name[64];
    char driver_version[32];
    void* driver_data;
    
    /* Device state */
    bool online;
    bool mounted;
    uint32_t mount_count;
    
    struct storage_device* next;
} storage_device_t;

/* Partition Information */
typedef struct partition {
    uint32_t id;                /* Partition ID */
    uint32_t device_id;         /* Parent device ID */
    uint32_t partition_number;  /* Partition number on device */
    
    char label[64];             /* Partition label */
    char uuid[40];              /* Partition UUID */
    uint8_t type_uuid[16];      /* Partition type UUID (GPT) */
    uint8_t partition_uuid[16]; /* Unique partition UUID (GPT) */
    
    filesystem_type_t fs_type;  /* Filesystem type */
    
    /* Geometry */
    uint64_t start_sector;      /* Starting sector */
    uint64_t end_sector;        /* Ending sector */
    uint64_t size_bytes;        /* Size in bytes */
    
    /* Flags */
    bool bootable;              /* Bootable flag */
    bool system;                /* System partition */
    bool hidden;                /* Hidden partition */
    bool read_only;             /* Read-only partition */
    
    /* Encryption */
    encryption_type_t encryption; /* Encryption type */
    uint8_t encryption_key[64]; /* Encryption key */
    uint32_t key_length;        /* Key length in bits */
    
    /* Mount information */
    bool mounted;
    char mount_point[256];      /* Mount point path */
    uint32_t mount_flags;       /* Mount flags */
    
    struct partition* next;
} partition_t;

/* Filesystem Statistics */
typedef struct filesystem_stats {
    uint64_t total_space;       /* Total filesystem space */
    uint64_t free_space;        /* Free space available */
    uint64_t used_space;        /* Used space */
    uint64_t available_space;   /* Available space to non-root */
    
    uint64_t total_inodes;      /* Total inodes */
    uint64_t free_inodes;       /* Free inodes */
    uint64_t used_inodes;       /* Used inodes */
    
    uint32_t block_size;        /* Filesystem block size */
    uint64_t total_blocks;      /* Total blocks */
    uint64_t free_blocks;       /* Free blocks */
    uint64_t reserved_blocks;   /* Reserved blocks */
    
    uint32_t fragment_size;     /* Fragment size */
    uint64_t total_fragments;   /* Total fragments */
    uint64_t free_fragments;    /* Free fragments */
    
    uint32_t max_filename_length; /* Maximum filename length */
    uint32_t max_path_length;   /* Maximum path length */
    
    /* Performance statistics */
    uint64_t reads_completed;
    uint64_t writes_completed;
    uint64_t bytes_read;
    uint64_t bytes_written;
    uint64_t read_time_ms;
    uint64_t write_time_ms;
    
} filesystem_stats_t;

/* Filesystem Operations */
typedef struct filesystem_ops {
    /* Superblock operations */
    status_t (*mount)(partition_t* partition, const char* mount_point, uint32_t flags, void* data);
    status_t (*unmount)(const char* mount_point, uint32_t flags);
    status_t (*sync)(const char* mount_point);
    status_t (*get_stats)(const char* mount_point, filesystem_stats_t* stats);
    
    /* Inode operations */
    status_t (*create)(const char* path, file_type_t type, uint32_t mode);
    status_t (*delete)(const char* path);
    status_t (*rename)(const char* old_path, const char* new_path);
    status_t (*link)(const char* target, const char* link_path);
    status_t (*symlink)(const char* target, const char* link_path);
    
    /* File operations */
    status_t (*open)(const char* path, uint32_t flags, void** file_handle);
    status_t (*close)(void* file_handle);
    ssize_t (*read)(void* file_handle, void* buffer, size_t count, uint64_t offset);
    ssize_t (*write)(void* file_handle, const void* buffer, size_t count, uint64_t offset);
    status_t (*truncate)(void* file_handle, uint64_t size);
    status_t (*flush)(void* file_handle);
    
    /* Directory operations */
    status_t (*mkdir)(const char* path, uint32_t mode);
    status_t (*rmdir)(const char* path);
    status_t (*readdir)(const char* path, void* buffer, size_t buffer_size);
    
    /* Attribute operations */
    status_t (*get_attr)(const char* path, void* attr_buffer);
    status_t (*set_attr)(const char* path, const void* attr_buffer);
    status_t (*get_extended_attr)(const char* path, const char* name, void* value, size_t size);
    status_t (*set_extended_attr)(const char* path, const char* name, const void* value, size_t size);
    
    /* Security operations */
    status_t (*set_permissions)(const char* path, uint32_t mode);
    status_t (*set_owner)(const char* path, uint32_t uid, uint32_t gid);
    status_t (*get_acl)(const char* path, void* acl_buffer, size_t buffer_size);
    status_t (*set_acl)(const char* path, const void* acl_buffer);
    
    /* Snapshot operations (for supporting filesystems) */
    status_t (*create_snapshot)(const char* path, const char* snapshot_name);
    status_t (*delete_snapshot)(const char* snapshot_name);
    status_t (*restore_snapshot)(const char* snapshot_name);
    status_t (*list_snapshots)(const char* path, void* buffer, size_t buffer_size);
    
} filesystem_ops_t;

/* Filesystem Driver */
typedef struct filesystem_driver {
    filesystem_type_t type;
    char name[32];
    char description[128];
    uint32_t version;
    
    /* Capabilities */
    struct {
        bool supports_encryption;
        bool supports_compression;
        bool supports_snapshots;
        bool supports_quotas;
        bool supports_acl;
        bool supports_extended_attr;
        bool supports_sparse_files;
        bool supports_hard_links;
        bool supports_symbolic_links;
        bool supports_case_sensitive;
        bool supports_case_preserving;
        bool supports_unicode;
        bool supports_journaling;
        bool supports_cow;          /* Copy-on-write */
        bool supports_deduplication;
        bool supports_checksums;
    } capabilities;
    
    /* Limits */
    uint64_t max_file_size;
    uint64_t max_filesystem_size;
    uint32_t max_filename_length;
    uint32_t max_path_length;
    uint32_t max_symlink_depth;
    
    /* Operations */
    filesystem_ops_t ops;
    
    struct filesystem_driver* next;
} filesystem_driver_t;

/* File Handle */
typedef struct file_handle {
    uint32_t id;                /* File handle ID */
    char path[512];             /* Full file path */
    filesystem_type_t fs_type;  /* Filesystem type */
    uint32_t flags;             /* Open flags */
    uint32_t mode;              /* File mode */
    
    /* Position and size */
    uint64_t position;          /* Current file position */
    uint64_t size;              /* File size */
    
    /* Caching */
    bool cache_enabled;
    void* cache_data;
    size_t cache_size;
    
    /* Lock information */
    bool locked;
    uint32_t lock_type;         /* Shared or exclusive */
    uint64_t lock_start;
    uint64_t lock_length;
    
    /* Filesystem-specific data */
    void* fs_private;
    
} file_handle_t;

/* Mount Point */
typedef struct mount_point {
    char path[256];             /* Mount point path */
    char device[64];            /* Device name */
    filesystem_type_t fs_type;  /* Filesystem type */
    uint32_t flags;             /* Mount flags */
    
    partition_t* partition;     /* Mounted partition */
    filesystem_driver_t* driver; /* Filesystem driver */
    
    /* Statistics */
    filesystem_stats_t stats;
    uint64_t mount_time;        /* Mount timestamp */
    uint32_t access_count;      /* Number of accesses */
    
    struct mount_point* next;
} mount_point_t;

/* Storage RAID Array */
typedef struct raid_array {
    uint32_t id;                /* RAID array ID */
    char name[64];              /* Array name */
    raid_type_t type;           /* RAID type */
    
    uint32_t device_count;      /* Number of devices */
    storage_device_t** devices; /* Array devices */
    
    uint64_t total_size;        /* Total array size */
    uint64_t usable_size;       /* Usable size */
    uint32_t stripe_size;       /* Stripe size in KB */
    
    /* Status */
    enum {
        RAID_STATUS_OPTIMAL,
        RAID_STATUS_DEGRADED,
        RAID_STATUS_REBUILDING,
        RAID_STATUS_FAILED
    } status;
    
    uint32_t rebuild_progress;  /* Rebuild progress percentage */
    
    struct raid_array* next;
} raid_array_t;

/* Storage Manager */
typedef struct storage_manager {
    bool initialized;
    uint32_t version;
    
    /* Devices */
    storage_device_t* devices;
    uint32_t device_count;
    
    /* Partitions */
    partition_t* partitions;
    uint32_t partition_count;
    
    /* Filesystems */
    filesystem_driver_t* filesystem_drivers;
    uint32_t fs_driver_count;
    
    /* Mount points */
    mount_point_t* mount_points;
    uint32_t mount_point_count;
    
    /* RAID arrays */
    raid_array_t* raid_arrays;
    uint32_t raid_array_count;
    
    /* File handles */
    file_handle_t* file_handles[MAX_OPEN_FILES];
    uint32_t next_file_id;
    
    /* Encryption contexts */
    void* encryption_contexts[MAX_ENCRYPTION_KEYS];
    uint32_t encryption_context_count;
    
    /* Compression contexts */
    void* compression_contexts[MAX_COMPRESSION_CONTEXTS];
    uint32_t compression_context_count;
    
    /* Performance statistics */
    struct {
        uint64_t total_reads;
        uint64_t total_writes;
        uint64_t bytes_read;
        uint64_t bytes_written;
        uint64_t read_time_total;
        uint64_t write_time_total;
        uint32_t current_iops;
        uint32_t peak_iops;
        float avg_latency_us;
        float cache_hit_ratio;
    } performance;
    
    /* Settings */
    bool enable_write_caching;
    bool enable_read_ahead;
    bool enable_compression;
    bool enable_encryption;
    uint32_t cache_size_mb;
    uint32_t read_ahead_kb;
    
} storage_manager_t;

/* Global storage manager */
extern storage_manager_t storage_manager;

/* Core Storage API */
status_t storage_init(void);
void storage_shutdown(void);
status_t storage_detect_devices(void);

/* Device Management */
storage_device_t* storage_get_device_by_id(uint32_t id);
storage_device_t* storage_get_device_by_name(const char* name);
status_t storage_scan_device(storage_device_t* device);
status_t storage_test_device(storage_device_t* device);
status_t storage_secure_erase(storage_device_t* device);

/* Partition Management */
status_t storage_create_partition_table(storage_device_t* device, bool use_gpt);
status_t storage_create_partition(storage_device_t* device, uint64_t start_mb, uint64_t size_mb, filesystem_type_t fs_type);
status_t storage_delete_partition(partition_t* partition);
status_t storage_resize_partition(partition_t* partition, uint64_t new_size_mb);
partition_t* storage_get_partition_by_id(uint32_t id);

/* Filesystem Management */
status_t storage_register_filesystem(filesystem_driver_t* driver);
status_t storage_unregister_filesystem(filesystem_type_t type);
filesystem_driver_t* storage_get_filesystem_driver(filesystem_type_t type);
status_t storage_format_partition(partition_t* partition, filesystem_type_t fs_type, const char* label);

/* Mount/Unmount Operations */
status_t storage_mount(const char* device, const char* mount_point, filesystem_type_t fs_type, uint32_t flags, const char* options);
status_t storage_unmount(const char* mount_point, uint32_t flags);
status_t storage_remount(const char* mount_point, uint32_t flags);
mount_point_t* storage_get_mount_point(const char* path);
status_t storage_get_mount_points(mount_point_t** mount_points, uint32_t* count);

/* File Operations */
status_t storage_open(const char* path, uint32_t flags, uint32_t mode, file_handle_t** handle);
status_t storage_close(file_handle_t* handle);
ssize_t storage_read(file_handle_t* handle, void* buffer, size_t count);
ssize_t storage_write(file_handle_t* handle, const void* buffer, size_t count);
status_t storage_seek(file_handle_t* handle, int64_t offset, int whence);
uint64_t storage_tell(file_handle_t* handle);
status_t storage_truncate(file_handle_t* handle, uint64_t size);
status_t storage_flush(file_handle_t* handle);
status_t storage_sync(void);

/* Directory Operations */
status_t storage_mkdir(const char* path, uint32_t mode);
status_t storage_rmdir(const char* path);
status_t storage_readdir(const char* path, void* entries, size_t* count);
status_t storage_chdir(const char* path);
char* storage_getcwd(char* buffer, size_t size);

/* File/Directory Information */
status_t storage_stat(const char* path, void* stat_buffer);
status_t storage_lstat(const char* path, void* stat_buffer);
status_t storage_fstat(file_handle_t* handle, void* stat_buffer);
status_t storage_access(const char* path, int mode);
bool storage_exists(const char* path);
bool storage_is_directory(const char* path);
bool storage_is_file(const char* path);

/* File Operations */
status_t storage_create_file(const char* path, uint32_t mode);
status_t storage_delete_file(const char* path);
status_t storage_rename(const char* old_path, const char* new_path);
status_t storage_copy_file(const char* src_path, const char* dst_path);
status_t storage_move_file(const char* src_path, const char* dst_path);
status_t storage_link(const char* target, const char* link_path);
status_t storage_symlink(const char* target, const char* link_path);

/* Permissions and Attributes */
status_t storage_chmod(const char* path, uint32_t mode);
status_t storage_chown(const char* path, uint32_t uid, uint32_t gid);
status_t storage_set_attributes(const char* path, uint32_t attributes);
status_t storage_get_attributes(const char* path, uint32_t* attributes);

/* Extended Attributes */
status_t storage_setxattr(const char* path, const char* name, const void* value, size_t size, int flags);
ssize_t storage_getxattr(const char* path, const char* name, void* value, size_t size);
ssize_t storage_listxattr(const char* path, char* list, size_t size);
status_t storage_removexattr(const char* path, const char* name);

/* RAID Management */
status_t storage_create_raid(raid_type_t type, storage_device_t** devices, uint32_t device_count, const char* name);
status_t storage_destroy_raid(raid_array_t* array);
status_t storage_add_raid_device(raid_array_t* array, storage_device_t* device);
status_t storage_remove_raid_device(raid_array_t* array, storage_device_t* device);
status_t storage_rebuild_raid(raid_array_t* array);

/* Encryption */
status_t storage_encrypt_partition(partition_t* partition, encryption_type_t type, const void* key, uint32_t key_length);
status_t storage_decrypt_partition(partition_t* partition, const void* key, uint32_t key_length);
status_t storage_change_encryption_key(partition_t* partition, const void* old_key, const void* new_key, uint32_t key_length);

/* Compression */
status_t storage_enable_compression(const char* path, compression_type_t type, uint32_t level);
status_t storage_disable_compression(const char* path);
status_t storage_set_compression_ratio(const char* path, uint32_t ratio);

/* Snapshots (for supporting filesystems) */
status_t storage_create_snapshot(const char* path, const char* snapshot_name);
status_t storage_delete_snapshot(const char* snapshot_name);
status_t storage_restore_snapshot(const char* snapshot_name);
status_t storage_list_snapshots(const char* path, char** snapshots, uint32_t* count);

/* Quotas */
status_t storage_set_user_quota(const char* path, uint32_t uid, uint64_t soft_limit, uint64_t hard_limit);
status_t storage_set_group_quota(const char* path, uint32_t gid, uint64_t soft_limit, uint64_t hard_limit);
status_t storage_get_quota_usage(const char* path, uint32_t uid, uint64_t* used, uint64_t* soft_limit, uint64_t* hard_limit);

/* Performance and Monitoring */
status_t storage_get_device_stats(storage_device_t* device, void* stats);
status_t storage_get_filesystem_stats(const char* mount_point, filesystem_stats_t* stats);
void storage_print_performance_stats(void);
status_t storage_benchmark_device(storage_device_t* device, void* results);

/* Maintenance */
status_t storage_check_filesystem(const char* device, bool fix_errors);
status_t storage_defragment_filesystem(const char* mount_point);
status_t storage_optimize_filesystem(const char* mount_point);
status_t storage_scrub_filesystem(const char* mount_point);

/* Utilities */
const char* storage_get_filesystem_name(filesystem_type_t type);
const char* storage_get_device_type_name(storage_device_type_t type);
uint64_t storage_parse_size_string(const char* size_str);
char* storage_format_size(uint64_t bytes, char* buffer, size_t buffer_size);
bool storage_path_is_absolute(const char* path);
char* storage_normalize_path(const char* path, char* buffer, size_t buffer_size);

#endif /* LIMITLESS_STORAGE_ENTERPRISE_H */