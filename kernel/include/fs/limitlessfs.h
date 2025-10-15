/**
 * LimitlessFS - Production Filesystem for LimitlessOS
 * 
 * Implements journaling filesystem with dcache/icache, advanced I/O scheduling,
 * file locking, extended attributes, quotas, and high-performance operations.
 * 
 * Copyright (c) 2024 LimitlessOS Project
 */

#ifndef __LIMITLESSFS_H__
#define __LIMITLESSFS_H__

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "mm/advanced.h"
#include "smp.h"

/* Filesystem constants */
#define LIMITLESSFS_MAGIC       0x4C495354  /* 'LIST' */
#define LIMITLESSFS_VERSION     1
#define LIMITLESSFS_BLOCK_SIZE  4096
#define LIMITLESSFS_MAX_NAME    255
#define LIMITLESSFS_MAX_SYMLINK 1024

/* Block group constants */
#define BLOCKS_PER_GROUP        32768
#define INODES_PER_GROUP        8192
#define GROUPS_PER_FLEX         16

/* Journal constants */
#define JOURNAL_MAGIC           0x4A4E4C00  /* 'JNL\0' */
#define JOURNAL_BLOCK_SIZE      4096
#define JOURNAL_MIN_SIZE        (1024 * 1024)  /* 1 MB */
#define JOURNAL_MAX_SIZE        (128 * 1024 * 1024)  /* 128 MB */
#define JOURNAL_COMMIT_INTERVAL 5000  /* 5 seconds */

/* Inode flags */
#define LFS_INODE_SECRM         0x00000001  /* Secure deletion */
#define LFS_INODE_UNRM          0x00000002  /* Undelete */
#define LFS_INODE_COMPR         0x00000004  /* Compress file */
#define LFS_INODE_SYNC          0x00000008  /* Synchronous updates */
#define LFS_INODE_IMMUTABLE     0x00000010  /* Immutable file */
#define LFS_INODE_APPEND        0x00000020  /* Append only */
#define LFS_INODE_NODUMP        0x00000040  /* No dump */
#define LFS_INODE_NOATIME       0x00000080  /* No atime updates */
#define LFS_INODE_DIRTY         0x00000100  /* Dirty */
#define LFS_INODE_COMPRBLK      0x00000200  /* Compressed blocks */
#define LFS_INODE_NOCOMPR       0x00000400  /* No compression */
#define LFS_INODE_ENCRYPT       0x00000800  /* Encrypted file */
#define LFS_INODE_INDEX         0x00001000  /* Hash-indexed directory */
#define LFS_INODE_IMAGIC        0x00002000  /* AFS directory */
#define LFS_INODE_JOURNAL_DATA  0x00004000  /* Journal file data */
#define LFS_INODE_NOTAIL        0x00008000  /* No tail merging */
#define LFS_INODE_DIRSYNC       0x00010000  /* Synchronous directory */
#define LFS_INODE_TOPDIR        0x00020000  /* Top level directory */
#define LFS_INODE_HUGE_FILE     0x00040000  /* Huge file */
#define LFS_INODE_EXTENTS       0x00080000  /* Use extents */
#define LFS_INODE_EA_INODE      0x00200000  /* Large EA inode */
#define LFS_INODE_EOFBLOCKS     0x00400000  /* Preallocated blocks */
#define LFS_INODE_INLINE_DATA   0x10000000  /* Inline data */

/* File types */
#define LFS_FT_UNKNOWN          0
#define LFS_FT_REG_FILE         1
#define LFS_FT_DIR              2
#define LFS_FT_CHRDEV           3
#define LFS_FT_BLKDEV           4
#define LFS_FT_FIFO             5
#define LFS_FT_SOCK             6
#define LFS_FT_SYMLINK          7
#define LFS_FT_MAX              8

/* Forward declarations */
typedef struct lfs_superblock lfs_superblock_t;
typedef struct lfs_group_desc lfs_group_desc_t;
typedef struct lfs_inode lfs_inode_t;
typedef struct lfs_dir_entry lfs_dir_entry_t;
typedef struct lfs_extent lfs_extent_t;
typedef struct lfs_journal lfs_journal_t;
typedef struct lfs_transaction lfs_transaction_t;

/**
 * Superblock structure
 */
struct lfs_superblock {
    uint32_t s_inodes_count;            /* Total inodes count */
    uint32_t s_blocks_count_lo;         /* Total blocks count (low 32 bits) */
    uint32_t s_r_blocks_count_lo;       /* Reserved blocks count (low 32 bits) */
    uint32_t s_free_blocks_count_lo;    /* Free blocks count (low 32 bits) */
    uint32_t s_free_inodes_count;       /* Free inodes count */
    uint32_t s_first_data_block;        /* First data block */
    uint32_t s_log_block_size;          /* Block size = 1024 << s_log_block_size */
    uint32_t s_log_cluster_size;        /* Cluster size = 1024 << s_log_cluster_size */
    uint32_t s_blocks_per_group;        /* Blocks per group */
    uint32_t s_clusters_per_group;      /* Clusters per group */
    uint32_t s_inodes_per_group;        /* Inodes per group */
    uint32_t s_mtime;                   /* Mount time */
    uint32_t s_wtime;                   /* Write time */
    uint16_t s_mnt_count;               /* Mount count */
    uint16_t s_max_mnt_count;           /* Max mount count */
    uint16_t s_magic;                   /* Magic signature */
    uint16_t s_state;                   /* File system state */
    uint16_t s_errors;                  /* Behavior on errors */
    uint16_t s_minor_rev_level;         /* Minor revision level */
    uint32_t s_lastcheck;               /* Last check time */
    uint32_t s_checkinterval;           /* Check interval */
    uint32_t s_creator_os;              /* Creator OS */
    uint32_t s_rev_level;               /* Revision level */
    uint16_t s_def_resuid;              /* Default reserved user ID */
    uint16_t s_def_resgid;              /* Default reserved group ID */
    
    /* Extended superblock fields */
    uint32_t s_first_ino;               /* First non-reserved inode */
    uint16_t s_inode_size;              /* Inode size */
    uint16_t s_block_group_nr;          /* Block group this SB is in */
    uint32_t s_feature_compat;          /* Compatible features */
    uint32_t s_feature_incompat;        /* Incompatible features */
    uint32_t s_feature_ro_compat;       /* Read-only compatible features */
    uint8_t s_uuid[16];                 /* Volume UUID */
    char s_volume_name[16];             /* Volume name */
    char s_last_mounted[64];            /* Last mount point */
    uint32_t s_algorithm_usage_bitmap;  /* Compression algorithms used */
    
    /* Performance hints */
    uint8_t s_prealloc_blocks;          /* Blocks to try to preallocate */
    uint8_t s_prealloc_dir_blocks;      /* Blocks to preallocate for dirs */
    uint16_t s_reserved_gdt_blocks;     /* Reserved GDT blocks for resize */
    
    /* Journaling support */
    uint8_t s_journal_uuid[16];         /* Journal UUID */
    uint32_t s_journal_inum;            /* Journal inode number */
    uint32_t s_journal_dev;             /* Journal device number */
    uint32_t s_last_orphan;             /* Head of orphan inode list */
    uint32_t s_hash_seed[4];            /* HTREE hash seed */
    uint8_t s_def_hash_version;         /* Default hash algorithm */
    uint8_t s_jnl_backup_type;          /* Journal backup type */
    uint16_t s_desc_size;               /* Group descriptor size */
    uint32_t s_default_mount_opts;      /* Default mount options */
    uint32_t s_first_meta_bg;           /* First metablock group */
    uint32_t s_mkfs_time;               /* Filesystem creation time */
    uint32_t s_jnl_blocks[17];          /* Backup journal blocks */
    
    /* 64-bit fields */
    uint32_t s_blocks_count_hi;         /* Blocks count (high 32 bits) */
    uint32_t s_r_blocks_count_hi;       /* Reserved blocks (high 32 bits) */
    uint32_t s_free_blocks_count_hi;    /* Free blocks (high 32 bits) */
    uint16_t s_min_extra_isize;         /* Minimum extra inode size */
    uint16_t s_want_extra_isize;        /* Desired extra inode size */
    uint32_t s_flags;                   /* Miscellaneous flags */
    uint16_t s_raid_stride;             /* RAID stride */
    uint16_t s_mmp_update_interval;     /* Multi-mount protection interval */
    uint64_t s_mmp_block;               /* Multi-mount protection block */
    uint32_t s_raid_stripe_width;       /* RAID stripe width */
    uint8_t s_log_groups_per_flex;      /* FLEX_BG group size */
    uint8_t s_checksum_type;            /* Metadata checksum algorithm */
    uint16_t s_reserved_pad;
    uint64_t s_kbytes_written;          /* Kilobytes written */
    uint32_t s_snapshot_inum;           /* Snapshot inode number */
    uint32_t s_snapshot_id;             /* Snapshot ID */
    uint64_t s_snapshot_r_blocks_count; /* Reserved blocks for snapshot */
    uint32_t s_snapshot_list;           /* Snapshot list head */
    uint32_t s_error_count;             /* Error count */
    uint32_t s_first_error_time;        /* First error time */
    uint32_t s_first_error_ino;         /* First error inode */
    uint64_t s_first_error_block;       /* First error block */
    uint8_t s_first_error_func[32];     /* First error function */
    uint32_t s_first_error_line;        /* First error line */
    uint32_t s_last_error_time;         /* Last error time */
    uint32_t s_last_error_ino;          /* Last error inode */
    uint32_t s_last_error_line;         /* Last error line */
    uint64_t s_last_error_block;        /* Last error block */
    uint8_t s_last_error_func[32];      /* Last error function */
    uint8_t s_mount_opts[64];           /* Mount options */
    uint32_t s_usr_quota_inum;          /* User quota inode */
    uint32_t s_grp_quota_inum;          /* Group quota inode */
    uint32_t s_overhead_clusters;       /* Overhead blocks/clusters */
    uint32_t s_backup_bgs[2];           /* Backup superblock locations */
    uint8_t s_encrypt_algos[4];         /* Encryption algorithms */
    uint8_t s_encrypt_pw_salt[16];      /* Salt for string2key algorithm */
    uint32_t s_lpf_ino;                 /* Location of lost+found inode */
    uint32_t s_prj_quota_inum;          /* Project quota inode */
    uint32_t s_checksum_seed;           /* CRC32c checksum seed */
    uint32_t s_reserved[98];            /* Padding to 1024 bytes */
    uint32_t s_checksum;                /* Superblock checksum */
};

/**
 * Block group descriptor
 */
struct lfs_group_desc {
    uint32_t bg_block_bitmap_lo;        /* Blocks bitmap block (low 32 bits) */
    uint32_t bg_inode_bitmap_lo;        /* Inodes bitmap block (low 32 bits) */
    uint32_t bg_inode_table_lo;         /* Inodes table block (low 32 bits) */
    uint16_t bg_free_blocks_count_lo;   /* Free blocks count (low 16 bits) */
    uint16_t bg_free_inodes_count_lo;   /* Free inodes count (low 16 bits) */
    uint16_t bg_used_dirs_count_lo;     /* Directories count (low 16 bits) */
    uint16_t bg_flags;                  /* Block group flags */
    uint32_t bg_exclude_bitmap_lo;      /* Exclude bitmap block (low 32 bits) */
    uint16_t bg_block_bitmap_csum_lo;   /* Block bitmap checksum (low 16 bits) */
    uint16_t bg_inode_bitmap_csum_lo;   /* Inode bitmap checksum (low 16 bits) */
    uint16_t bg_itable_unused_lo;       /* Unused inodes count (low 16 bits) */
    uint16_t bg_checksum;               /* Group descriptor checksum */
    
    /* 64-bit fields */
    uint32_t bg_block_bitmap_hi;        /* Blocks bitmap block (high 32 bits) */
    uint32_t bg_inode_bitmap_hi;        /* Inodes bitmap block (high 32 bits) */
    uint32_t bg_inode_table_hi;         /* Inodes table block (high 32 bits) */
    uint16_t bg_free_blocks_count_hi;   /* Free blocks count (high 16 bits) */
    uint16_t bg_free_inodes_count_hi;   /* Free inodes count (high 16 bits) */
    uint16_t bg_used_dirs_count_hi;     /* Directories count (high 16 bits) */
    uint16_t bg_itable_unused_hi;       /* Unused inodes count (high 16 bits) */
    uint32_t bg_exclude_bitmap_hi;      /* Exclude bitmap block (high 32 bits) */
    uint16_t bg_block_bitmap_csum_hi;   /* Block bitmap checksum (high 16 bits) */
    uint16_t bg_inode_bitmap_csum_hi;   /* Inode bitmap checksum (high 16 bits) */
    uint32_t bg_reserved;
};

/**
 * Inode structure
 */
struct lfs_inode {
    uint16_t i_mode;                    /* File mode */
    uint16_t i_uid;                     /* Owner UID (low 16 bits) */
    uint32_t i_size_lo;                 /* Size (low 32 bits) */
    uint32_t i_atime;                   /* Access time */
    uint32_t i_ctime;                   /* Creation time */
    uint32_t i_mtime;                   /* Modification time */
    uint32_t i_dtime;                   /* Deletion time */
    uint16_t i_gid;                     /* Group GID (low 16 bits) */
    uint16_t i_links_count;             /* Links count */
    uint32_t i_blocks_lo;               /* Blocks count (low 32 bits) */
    uint32_t i_flags;                   /* File flags */
    
    union {
        struct {
            uint32_t l_i_version;       /* Version (Linux) */
        } linux1;
        struct {
            uint32_t h_i_translator;    /* Translator (Hurd) */
        } hurd1;
        struct {
            uint32_t m_i_reserved1;     /* Reserved (Masix) */
        } masix1;
    } osd1;
    
    uint32_t i_block[15];               /* Pointers to blocks */
    uint32_t i_generation;              /* File version */
    uint32_t i_file_acl_lo;             /* File ACL (low 32 bits) */
    uint32_t i_size_high;               /* Size (high 32 bits) */
    uint32_t i_obso_faddr;              /* Obsoleted fragment address */
    
    union {
        struct {
            uint16_t l_i_blocks_high;   /* Blocks count (high 16 bits) */
            uint16_t l_i_file_acl_high; /* File ACL (high 16 bits) */
            uint16_t l_i_uid_high;      /* Owner UID (high 16 bits) */
            uint16_t l_i_gid_high;      /* Group GID (high 16 bits) */
            uint16_t l_i_checksum_lo;   /* Checksum (low 16 bits) */
            uint16_t l_i_reserved;
        } linux2;
        struct {
            uint16_t h_i_reserved1;
            uint16_t h_i_mode_high;
            uint16_t h_i_uid_high;
            uint16_t h_i_gid_high;
            uint32_t h_i_author;
        } hurd2;
        struct {
            uint16_t h_i_reserved1;
            uint16_t m_i_file_acl_high;
            uint32_t m_i_reserved2[2];
        } masix2;
    } osd2;
    
    uint16_t i_extra_isize;             /* Extra inode size */
    uint16_t i_checksum_hi;             /* Checksum (high 16 bits) */
    uint32_t i_ctime_extra;             /* Creation time (extra) */
    uint32_t i_mtime_extra;             /* Modification time (extra) */
    uint32_t i_atime_extra;             /* Access time (extra) */
    uint32_t i_crtime;                  /* Creation time */
    uint32_t i_crtime_extra;            /* Creation time (extra) */
    uint32_t i_version_hi;              /* Version (high 32 bits) */
    uint32_t i_projid;                  /* Project ID */
};

/**
 * Directory entry structure
 */
struct lfs_dir_entry {
    uint32_t inode;                     /* Inode number */
    uint16_t rec_len;                   /* Directory entry length */
    uint8_t name_len;                   /* Name length */
    uint8_t file_type;                  /* File type */
    char name[LFS_MAX_NAME];            /* File name */
};

/**
 * Extent structure
 */
struct lfs_extent {
    uint32_t ee_block;                  /* First logical block */
    uint16_t ee_len;                    /* Number of blocks */
    uint16_t ee_start_hi;               /* Physical block (high 16 bits) */
    uint32_t ee_start_lo;               /* Physical block (low 32 bits) */
};

/**
 * Extent tree header
 */
typedef struct {
    uint16_t eh_magic;                  /* Magic number */
    uint16_t eh_entries;                /* Number of valid entries */
    uint16_t eh_max;                    /* Maximum entries */
    uint16_t eh_depth;                  /* Tree depth */
    uint32_t eh_generation;             /* Generation */
} lfs_extent_header_t;

/**
 * Journal superblock
 */
typedef struct {
    uint32_t s_header_magic;            /* Journal magic */
    uint32_t s_blocktype;               /* Journal block type */
    uint32_t s_sequence;                /* Sequence number */
    uint32_t s_start;                   /* First commit ID */
    uint32_t s_errno;                   /* Error number */
    uint32_t s_feature_compat;          /* Compatible features */
    uint32_t s_feature_incompat;        /* Incompatible features */
    uint32_t s_feature_ro_compat;       /* Read-only compatible features */
    uint8_t s_uuid[16];                 /* Journal UUID */
    uint32_t s_nr_users;                /* Number of filesystems using journal */
    uint32_t s_dynsuper;                /* Dynamic superblock flag */
    uint32_t s_max_transaction;         /* Maximum transaction size */
    uint32_t s_max_trans_data;          /* Maximum data per transaction */
    uint32_t s_checksum_type;           /* Checksum algorithm */
    uint32_t s_checksum_size;           /* Checksum size */
    uint32_t s_num_fc_blocks;           /* Fast commit blocks */
    uint8_t s_padding[1020];            /* Padding */
    uint32_t s_checksum;                /* Superblock checksum */
} lfs_journal_superblock_t;

/**
 * Journal transaction
 */
struct lfs_transaction {
    uint32_t t_tid;                     /* Transaction ID */
    uint32_t t_state;                   /* Transaction state */
    uint64_t t_log_start;               /* Log start block */
    uint32_t t_nr_buffers;              /* Number of buffers */
    uint32_t t_outstanding_credits;     /* Outstanding credits */
    spinlock_t t_handle_lock;           /* Handle lock */
    atomic_t t_updates;                 /* Update count */
    atomic_t t_handle_count;            /* Handle count */
    struct list_head t_inode_list;      /* Modified inodes */
    struct list_head t_buffers;         /* Modified buffers */
    unsigned long t_expires;            /* Expiration time */
    lfs_transaction_t *t_checkpoint_list; /* Checkpoint list */
    lfs_transaction_t *t_checkpoint_io_list; /* Checkpoint I/O list */
    wait_queue_head_t t_wait;           /* Wait queue */
};

/**
 * Directory cache entry
 */
typedef struct dcache_entry {
    char *name;                         /* Directory name */
    size_t name_len;                    /* Name length */
    uint32_t parent_ino;                /* Parent inode number */
    uint32_t inode_no;                  /* Inode number */
    uint32_t hash;                      /* Name hash */
    struct dcache_entry *parent;        /* Parent entry */
    struct dcache_entry *next_hash;     /* Hash chain */
    struct dcache_entry *next_lru;      /* LRU chain */
    struct dcache_entry *prev_lru;      /* LRU chain */
    atomic_t ref_count;                 /* Reference count */
    unsigned long flags;                /* Entry flags */
    uint64_t last_used;                 /* Last access time */
} dcache_entry_t;

/**
 * Inode cache entry
 */
typedef struct icache_entry {
    uint32_t inode_no;                  /* Inode number */
    lfs_inode_t inode;                  /* Cached inode */
    void *private_data;                 /* Private data */
    struct icache_entry *next_hash;     /* Hash chain */
    struct icache_entry *next_lru;      /* LRU chain */
    struct icache_entry *prev_lru;      /* LRU chain */
    atomic_t ref_count;                 /* Reference count */
    unsigned long flags;                /* Entry flags */
    uint64_t last_used;                 /* Last access time */
    spinlock_t lock;                    /* Entry lock */
} icache_entry_t;

/**
 * File lock structure
 */
typedef struct file_lock {
    uint32_t fl_owner;                  /* Lock owner */
    uint32_t fl_pid;                    /* Process ID */
    uint8_t fl_type;                    /* Lock type */
    uint8_t fl_flags;                   /* Lock flags */
    loff_t fl_start;                    /* Start offset */
    loff_t fl_end;                      /* End offset */
    struct file_lock *fl_next;          /* Next lock */
    struct file_lock *fl_block;         /* Blocked locks */
    wait_queue_head_t fl_wait;          /* Wait queue */
} file_lock_t;

/**
 * Extended attribute structure
 */
typedef struct {
    uint8_t e_name_len;                 /* Name length */
    uint8_t e_name_index;               /* Name index */
    uint16_t e_value_offs;              /* Value offset */
    uint32_t e_value_size;              /* Value size */
    uint32_t e_hash;                    /* Hash value */
    char e_name[0];                     /* Attribute name */
} lfs_xattr_entry_t;

/**
 * Quota structure
 */
typedef struct {
    uint32_t dq_id;                     /* User/group ID */
    uint64_t dqb_bhardlimit;            /* Block hard limit */
    uint64_t dqb_bsoftlimit;            /* Block soft limit */
    uint64_t dqb_curspace;              /* Current space usage */
    uint64_t dqb_ihardlimit;            /* Inode hard limit */
    uint64_t dqb_isoftlimit;            /* Inode soft limit */
    uint64_t dqb_curinodes;             /* Current inode usage */
    uint64_t dqb_btime;                 /* Block grace time */
    uint64_t dqb_itime;                 /* Inode grace time */
} lfs_quota_t;

/* Function declarations */

/* Filesystem initialization */
int lfs_init(void);
int lfs_mount(const char *device, const char *mountpoint, uint32_t flags);
int lfs_umount(const char *mountpoint, uint32_t flags);
int lfs_mkfs(const char *device, size_t size, const char *label);

/* Inode operations */
lfs_inode_t *lfs_iget(uint32_t ino);
void lfs_iput(lfs_inode_t *inode);
int lfs_write_inode(lfs_inode_t *inode);
int lfs_delete_inode(lfs_inode_t *inode);
uint32_t lfs_new_inode(uint16_t mode, uint32_t uid, uint32_t gid);

/* File operations */
int lfs_create(const char *path, uint16_t mode, uint32_t uid, uint32_t gid);
int lfs_open(const char *path, int flags, uint16_t mode);
int lfs_close(int fd);
ssize_t lfs_read(int fd, void *buf, size_t count);
ssize_t lfs_write(int fd, const void *buf, size_t count);
loff_t lfs_lseek(int fd, loff_t offset, int whence);
int lfs_truncate(const char *path, loff_t length);
int lfs_unlink(const char *path);

/* Directory operations */
int lfs_mkdir(const char *path, uint16_t mode);
int lfs_rmdir(const char *path);
int lfs_readdir(int fd, struct dirent *dirp, size_t count);
int lfs_rename(const char *oldpath, const char *newpath);
int lfs_link(const char *oldpath, const char *newpath);
int lfs_symlink(const char *target, const char *linkpath);
ssize_t lfs_readlink(const char *path, char *buf, size_t bufsiz);

/* Cache management */
int dcache_init(void);
dcache_entry_t *dcache_lookup(const char *name, dcache_entry_t *parent);
void dcache_add(dcache_entry_t *entry);
void dcache_remove(dcache_entry_t *entry);
void dcache_invalidate(dcache_entry_t *entry);

int icache_init(void);
icache_entry_t *icache_lookup(uint32_t ino);
void icache_add(icache_entry_t *entry);
void icache_remove(icache_entry_t *entry);
void icache_sync(void);

/* Journal operations */
int lfs_journal_init(void);
lfs_transaction_t *lfs_journal_start(int nblocks);
int lfs_journal_get_write_access(lfs_transaction_t *handle, void *bh);
int lfs_journal_dirty_metadata(lfs_transaction_t *handle, void *bh);
int lfs_journal_stop(lfs_transaction_t *handle);
int lfs_journal_force_commit(void);
int lfs_journal_flush(void);

/* File locking */
int lfs_lock_file(int fd, int cmd, struct flock *fl);
int lfs_fcntl_setlk(int fd, int cmd, struct flock *fl);
void lfs_locks_remove_posix(struct file *filp, fl_owner_t owner);

/* Extended attributes */
ssize_t lfs_getxattr(const char *path, const char *name, void *value, size_t size);
int lfs_setxattr(const char *path, const char *name, const void *value, size_t size, int flags);
ssize_t lfs_listxattr(const char *path, char *list, size_t size);
int lfs_removexattr(const char *path, const char *name);

/* Quota management */
int lfs_quota_on(const char *path, int type, int format_id, const char *quota_file);
int lfs_quota_off(const char *path, int type);
int lfs_get_quota(int type, qid_t id, struct dqblk *dq);
int lfs_set_quota(int type, qid_t id, struct dqblk *dq);

/* Block allocation */
uint64_t lfs_new_block(lfs_inode_t *inode, uint32_t goal);
void lfs_free_block(uint64_t block);
uint64_t lfs_new_blocks(lfs_inode_t *inode, uint32_t goal, uint32_t count);
void lfs_free_blocks(uint64_t block, uint32_t count);

/* Extent management */
int lfs_ext_get_blocks(lfs_inode_t *inode, uint32_t lblk, uint32_t max_blocks,
                      uint64_t *pblk, uint32_t *len, int create);
int lfs_ext_truncate(lfs_inode_t *inode);
int lfs_ext_punch_hole(lfs_inode_t *inode, loff_t offset, loff_t len);

/* I/O operations */
int lfs_submit_bio(int rw, uint64_t sector, void *buffer, size_t size);
void lfs_end_bio(struct bio *bio, int error);
int lfs_sync_dirty_buffer(void *bh);
void lfs_mark_buffer_dirty(void *bh);

/* Filesystem utilities */
int lfs_check_dir_entry(const char *function, lfs_inode_t *dir,
                       lfs_dir_entry_t *de, void *bh, uint32_t offset);
uint32_t lfs_dx_hash(const char *name, int len);
void lfs_update_dynamic_rev(struct super_block *sb);

/* Debug and statistics */
void lfs_show_stats(void);
void lfs_debug_dump_inode(lfs_inode_t *inode);
void lfs_debug_dump_extent_tree(lfs_inode_t *inode);
int lfs_check_filesystem(void);

#endif /* __LIMITLESSFS_H__ */