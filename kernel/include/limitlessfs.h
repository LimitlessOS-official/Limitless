/*
 * LimitlessFS Header File
 * Definitions and structures for the LimitlessFS filesystem
 */

#ifndef _LIMITLESSFS_H
#define _LIMITLESSFS_H

#include <linux/fs.h>
#include <linux/buffer_head.h>
#include <linux/mutex.h>
#include <linux/workqueue.h>
#include <linux/rbtree.h>
#include <linux/crypto.h>
#include <linux/atomic.h>

// Forward declarations
struct limitlessfs_context;
struct limitlessfs_inode;
struct limitlessfs_superblock;

// Filesystem feature flags
#define LIMITLESSFS_FEATURE_COMPRESSION   (1 << 0)
#define LIMITLESSFS_FEATURE_ENCRYPTION    (1 << 1)
#define LIMITLESSFS_FEATURE_JOURNALING    (1 << 2)
#define LIMITLESSFS_FEATURE_SNAPSHOTS     (1 << 3)
#define LIMITLESSFS_FEATURE_CHECKSUMS     (1 << 4)
#define LIMITLESSFS_FEATURE_DEDUP         (1 << 5)
#define LIMITLESSFS_FEATURE_QUOTAS        (1 << 6)
#define LIMITLESSFS_FEATURE_AI_OPTIMIZE   (1 << 7)

// Compression algorithms
#define LIMITLESSFS_COMP_NONE     0
#define LIMITLESSFS_COMP_LZ4      1
#define LIMITLESSFS_COMP_ZSTD     2
#define LIMITLESSFS_COMP_BROTLI   3

// Encryption algorithms
#define LIMITLESSFS_CRYPT_NONE    0
#define LIMITLESSFS_CRYPT_AES256  1
#define LIMITLESSFS_CRYPT_CHACHA20 2

// Function prototypes

// Core filesystem operations
int limitlessfs_fill_super(struct super_block *sb, void *data, int silent);
struct inode *limitlessfs_iget(struct super_block *sb, unsigned long ino);
int limitlessfs_write_inode(struct inode *inode, struct writeback_control *wbc);

// Block and extent management
struct buffer_head *limitlessfs_read_inode_block(struct super_block *sb, 
                                                 unsigned long ino);
struct buffer_head *limitlessfs_read_data_block(struct inode *inode, 
                                               sector_t block);
struct limitlessfs_inode *limitlessfs_get_inode_from_block(struct buffer_head *bh,
                                                          unsigned long ino);

// Compression functions
int limitlessfs_init_compression(struct limitlessfs_context *ctx);
void limitlessfs_cleanup_compression(struct limitlessfs_context *ctx);
ssize_t limitlessfs_read_compressed(struct kiocb *iocb, struct iov_iter *iter);
ssize_t limitlessfs_write_compressed(struct kiocb *iocb, struct iov_iter *iter);
bool limitlessfs_inode_is_compressed(struct inode *inode);

// Encryption functions
int limitlessfs_init_encryption(struct limitlessfs_context *ctx);
void limitlessfs_cleanup_encryption(struct limitlessfs_context *ctx);
int limitlessfs_encrypt_block(struct limitlessfs_context *ctx,
                             struct page *src_page, struct page *dst_page);
int limitlessfs_decrypt_block(struct limitlessfs_context *ctx,
                             struct page *src_page, struct page *dst_page);

// Journal functions
int limitlessfs_init_journal(struct limitlessfs_context *ctx);
void limitlessfs_cleanup_journal(struct limitlessfs_context *ctx);
int limitlessfs_journal_write_start_record(struct limitlessfs_context *ctx,
                                          uint64_t transaction_id);
int limitlessfs_journal_write_commit_record(struct limitlessfs_context *ctx,
                                           uint64_t transaction_id);
int limitlessfs_journal_flush(struct limitlessfs_context *ctx);

// Snapshot functions
int limitlessfs_init_snapshots(struct limitlessfs_context *ctx);
void limitlessfs_cleanup_snapshots(struct limitlessfs_context *ctx);
int limitlessfs_cow_create_root(struct limitlessfs_context *ctx,
                               struct limitlessfs_snapshot *snapshot);
void limitlessfs_add_snapshot_to_tree(struct limitlessfs_snapshots *snapshots,
                                     struct limitlessfs_snapshot *snapshot);

// Cache management
int limitlessfs_init_caches(struct limitlessfs_context *ctx);
void limitlessfs_cleanup_caches(struct limitlessfs_context *ctx);
int limitlessfs_init_global_caches(void);
void limitlessfs_destroy_global_caches(void);

// AI optimization functions
int ai_fs_optimizer_init(struct ai_fs_optimizer *optimizer);
void ai_fs_optimizer_cleanup(struct ai_fs_optimizer *optimizer);
void ai_init_inode_profile(struct inode *inode, struct limitlessfs_inode *raw_inode);
int ai_predict_and_prefetch(struct inode *inode, loff_t pos, size_t len);
void ai_learn_access_pattern(struct inode *inode, loff_t pos, size_t len, bool write);
void ai_analyze_filesystem_patterns(struct limitlessfs_context *ctx);
void ai_optimize_block_allocation(struct limitlessfs_context *ctx);
void ai_optimize_cache_settings(struct limitlessfs_context *ctx);
void ai_predict_future_io(struct limitlessfs_context *ctx);

// Utility functions
bool limitlessfs_verify_checksum(const void *data, size_t len, uint32_t checksum);
uint32_t limitlessfs_calculate_checksum(const void *data, size_t len);
void limitlessfs_cleanup_subsystems(struct limitlessfs_context *ctx);

// Inode operations
extern struct inode_operations limitlessfs_inode_ops;
extern struct inode_operations limitlessfs_symlink_inode_ops;
extern struct file_operations limitlessfs_file_ops;
extern struct file_operations limitlessfs_dir_ops;
extern struct address_space_operations limitlessfs_aops;

#endif /* _LIMITLESSFS_H */