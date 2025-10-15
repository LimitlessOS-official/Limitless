#pragma once
#include "kernel.h"
#include "vfs.h"

#define EXT4_SUPER_MAGIC 0xEF53
#define EXT4_INODE_FLAG_EXTENTS 0x00080000
#define EXT4_NDIR_BLOCKS 12
#define EXT4_IND_BLOCK 12
#define EXT4_DIND_BLOCK 13
#define EXT4_TIND_BLOCK 14

#pragma pack(push,1)
typedef struct {
	u32 inodes_count;
	u32 blocks_count_lo;
	u32 r_blocks_count_lo;
	u32 free_blocks_count_lo;
	u32 free_inodes_count;
	u32 first_data_block;
	u32 log_block_size;
	u32 log_cluster_size;
	u32 blocks_per_group;
	u32 clusters_per_group;
	u32 inodes_per_group;
	u32 mtime;
	u32 wtime;
	u16 mnt_count;
	u16 max_mnt_count;
	u16 magic;
	u16 state;
	u16 errors;
	u16 minor_rev_level;
	u32 lastcheck;
	u32 checkinterval;
	u32 creator_os;
	u32 rev_level;
	u16 def_resuid;
	u16 def_resgid;
	u32 first_ino;
	u16 inode_size;
	u16 block_group_nr;
	u32 feature_compat;
	u32 feature_incompat;
	u32 feature_ro_compat;
	u8  uuid[16];
	char volume_name[16];
	char last_mounted[64];
	u32 algorithm_usage_bitmap;
	/* ... fields omitted ... */
} ext4_superblock_t;

typedef struct {
	u32 block_bitmap_lo;
	u32 inode_bitmap_lo;
	u32 inode_table_lo;
	u16 free_blocks_count_lo;
	u16 free_inodes_count_lo;
	u16 used_dirs_count_lo;
	u16 pad;
	u32 reserved[3];
} ext4_group_desc_t;

typedef struct {
	u16 mode;
	u16 uid;
	u32 size_lo;
	u32 atime;
	u32 ctime;
	u32 mtime;
	u32 dtime;
	u16 gid;
	u16 links_count;
	u32 blocks_lo;
	u32 flags;
	u32 osd1;
	u32 block[15];
	u32 generation;
	u32 file_acl_lo;
	u32 size_high;
	u32 obso_faddr;
	u8  osd2[12];
} ext4_inode_t;

typedef struct {
	u16 magic;
	u16 entries;
	u16 max;
	u16 depth;
	u32 gen;
} ext4_extent_header_t;

typedef struct {
	u32 block;
	u16 len;
	u16 start_hi;
	u32 start_lo;
} ext4_extent_t;

typedef struct {
	u32 block;
	u32 leaf_lo;
	u16 leaf_hi;
	u16 unused;
} ext4_extent_idx_t;

typedef struct {
	u32 ino;
	u16 rec_len;
	u8  name_len;
	u8  file_type;
	char name[];
} ext4_dir_entry_t;
#pragma pack(pop)

typedef struct {
	block_dev_t* bdev;
	u32 block_size;
	u32 blocks_per_group;
	u32 inodes_per_group;
	u32 inodes_count;
	u32 first_data_block;
	u32 desc_per_block;
	u64 groups;
	u64 gdt_start_block;
	u16 inode_size;
	u64 journal_start_block;
	u32 journal_block_count;
	u32 journal_head;
} ext4_sb_info;

typedef struct {
	ext4_sb_info* sbi;
	u64 ino;
	ext4_inode_t raw;
	int is_dir;
} ext4_inode_wrap;

int ext4_register(void);
int ext4_write_file(const char* path, const void* buf, size_t len, u64 off);
int ext4_create_file(const char* path, u32 mode);
int run_ext4_extent_tests(void);

#ifdef CONFIG_FS_TESTS
typedef struct {
	u32 logical;
	u32 len;
	u64 phys;
} ext4_debug_extent_t;
int ext4_debug_get_extent_depth(const char* path);
int ext4_debug_list_extents(const char* path, ext4_debug_extent_t* out, int max);
int ext4_debug_get_index_entries(const char* path);
int ext4_debug_fragment_append(const char* path, int runs, u32 block_size_hint);
#endif

// ---- EXT4 STUB HELPERS ----
#ifndef EXT4_STUB_HELPERS
#define EXT4_STUB_HELPERS
static inline int ext4_free_block(ext4_sb_info* sbi, u32 blk) { (void)sbi; (void)blk; return 0; }
static inline int ext4_extent_shrink_to(ext4_inode_wrap* iw, u64 new_size) { (void)iw; (void)new_size; return 0; }
static inline int ext4_store_inode(ext4_inode_wrap* iw) { (void)iw; return 0; }
static inline int ext4_inode_write_extents(ext4_inode_wrap* iw, u64 off, const void* buf, size_t len) { (void)iw; (void)off; (void)buf; (void)len; return 0; }
static inline void journal_replay(ext4_sb_info* sbi) { (void)sbi; }
static inline int bmap_ext4(ext4_inode_wrap* iw, u32 lblock, u64* out_pblk) { (void)iw; (void)lblock; if (out_pblk) *out_pblk = 0; return 0; }
static inline int ext4_inode_write(ext4_inode_wrap* iw, u64 off, const void* buf, size_t len) { (void)iw; (void)off; (void)buf; (void)len; return 0; }
static inline int ext4_read_block(ext4_sb_info* sbi, u64 block, void* buf) { (void)sbi; (void)block; (void)buf; return 0; }
#endif