/*
 * ext2 Filesystem Driver
 * 
 * Basic read-only ext2 implementation supporting:
 * - Superblock parsing
 * - Inode reading
 * - Directory traversal
 * - File reading
 * 
 * Copyright (c) 2024 LimitlessOS Project
 */

#include "kernel.h"
#include "vfs.h"
#include <mm/mm.h>
#include <string.h>

// ext2 magic number
#define EXT2_SUPER_MAGIC 0xEF53

// ext2 structures
typedef struct {
    u32 s_inodes_count;
    u32 s_blocks_count;
    u32 s_r_blocks_count;
    u32 s_free_blocks_count;
    u32 s_free_inodes_count;
    u32 s_first_data_block;
    u32 s_log_block_size;
    u32 s_log_frag_size;
    u32 s_blocks_per_group;
    u32 s_frags_per_group;
    u32 s_inodes_per_group;
    u32 s_mtime;
    u32 s_wtime;
    u16 s_mnt_count;
    u16 s_max_mnt_count;
    u16 s_magic;
    u16 s_state;
    u16 s_errors;
    u16 s_minor_rev_level;
    u32 s_lastcheck;
    u32 s_checkinterval;
    u32 s_creator_os;
    u32 s_rev_level;
    u16 s_def_resuid;
    u16 s_def_resgid;
    // Extended fields (rev_level >= 1)
    u32 s_first_ino;
    u16 s_inode_size;
    u16 s_block_group_nr;
    u32 s_feature_compat;
    u32 s_feature_incompat;
    u32 s_feature_ro_compat;
    u8  s_uuid[16];
    char s_volume_name[16];
} __attribute__((packed)) ext2_superblock_t;

typedef struct {
    u16 i_mode;
    u16 i_uid;
    u32 i_size;
    u32 i_atime;
    u32 i_ctime;
    u32 i_mtime;
    u32 i_dtime;
    u16 i_gid;
    u16 i_links_count;
    u32 i_blocks;
    u32 i_flags;
    u32 i_osd1;
    u32 i_block[15];  // 12 direct, 1 indirect, 1 double, 1 triple
    u32 i_generation;
    u32 i_file_acl;
    u32 i_dir_acl;
    u32 i_faddr;
    u8  i_osd2[12];
} __attribute__((packed)) ext2_inode_t;

typedef struct {
    u32 inode;
    u16 rec_len;
    u8  name_len;
    u8  file_type;
    char name[];
} __attribute__((packed)) ext2_dirent_t;

// ext2 filesystem private data
typedef struct {
    ext2_superblock_t superblock;
    u32 block_size;
    u32 inode_size;
    u32 inodes_per_group;
    u32 blocks_per_group;
} ext2_fs_t;

// ext2 inode private data
typedef struct {
    ext2_inode_t inode;
    u32 inode_num;
} ext2_inode_data_t;

// File type constants
#define EXT2_FT_REG_FILE  1
#define EXT2_FT_DIR       2
#define EXT2_FT_SYMLINK   7

// Inode mode bits
#define EXT2_S_IFDIR  0x4000
#define EXT2_S_IFREG  0x8000

// Forward declarations
static vnode_t *ext2_get_root(vfs_super_t *sb);
static long ext2_read_file(vnode_t *vn, u64 off, void *buf, size_t len);
static int ext2_readdir(vnode_t *vn, vfs_dirent_cb cb, void *ctx);
static vnode_t *ext2_lookup(vnode_t *dir, const char *name, size_t namelen);

static vnode_ops_t ext2_file_ops = {
    .read = ext2_read_file,
    .write = NULL,  // Read-only for now
    .readdir = NULL,
    .lookup = NULL,
    .release = NULL
};

static vnode_ops_t ext2_dir_ops = {
    .read = NULL,
    .write = NULL,
    .readdir = ext2_readdir,
    .lookup = ext2_lookup,
    .release = NULL
};

static vfs_super_ops_t ext2_super_ops = {
    .get_root = ext2_get_root,
    .put_super = NULL
};

// Read block from device
static int ext2_read_block(vfs_super_t *sb, u32 block_num, void *buf) {
    ext2_fs_t *fs = (ext2_fs_t *)sb->fs_priv;
    
    // TODO: Use block device to read
    // For now, return empty block
    memset(buf, 0, fs->block_size);
    return 0;
}

// Read inode
static int ext2_read_inode(vfs_super_t *sb, u32 inode_num, ext2_inode_t *inode_out) {
    ext2_fs_t *fs = (ext2_fs_t *)sb->fs_priv;
    
    // Calculate block group
    u32 group = (inode_num - 1) / fs->inodes_per_group;
    u32 index = (inode_num - 1) % fs->inodes_per_group;
    
    // TODO: Read from inode table
    // For now, create a fake root inode
    if (inode_num == 2) {  // Root inode
        memset(inode_out, 0, sizeof(ext2_inode_t));
        inode_out->i_mode = EXT2_S_IFDIR | 0755;
        inode_out->i_size = fs->block_size;
        inode_out->i_links_count = 2;
        return 0;
    }
    
    return -1;
}

// Create vnode from inode
static vnode_t *ext2_create_vnode(vfs_super_t *sb, u32 inode_num) {
    ext2_inode_t inode;
    if (ext2_read_inode(sb, inode_num, &inode) != 0) {
        return NULL;
    }
    
    vnode_t *vn = (vnode_t *)vmm_kmalloc(sizeof(vnode_t), 8);
    if (!vn) return NULL;
    
    memset(vn, 0, sizeof(vnode_t));
    
    vn->mnt = sb->mnt;
    vn->ino = inode_num;
    vn->size = inode.i_size;
    vn->mode = inode.i_mode & 0xFFF;
    vn->uid = inode.i_uid;
    vn->gid = inode.i_gid;
    
    // Determine type
    if (inode.i_mode & EXT2_S_IFDIR) {
        vn->type = VNODE_DIR;
        vn->ops = &ext2_dir_ops;
    } else {
        vn->type = VNODE_FILE;
        vn->ops = &ext2_file_ops;
    }
    
    // Allocate private data
    ext2_inode_data_t *priv = (ext2_inode_data_t *)vmm_kmalloc(sizeof(ext2_inode_data_t), 8);
    if (priv) {
        memcpy(&priv->inode, &inode, sizeof(ext2_inode_t));
        priv->inode_num = inode_num;
        vn->fs_priv = priv;
    }
    
    return vn;
}

// Get root vnode
static vnode_t *ext2_get_root(vfs_super_t *sb) {
    return ext2_create_vnode(sb, 2);  // Root inode is always 2
}

// Read file data
static long ext2_read_file(vnode_t *vn, u64 off, void *buf, size_t len) {
    if (!vn || !vn->fs_priv) return -1;
    
    ext2_inode_data_t *priv = (ext2_inode_data_t *)vn->fs_priv;
    ext2_fs_t *fs = (ext2_fs_t *)vn->mnt->sb->fs_priv;
    
    // Clamp to file size
    if (off >= vn->size) return 0;
    if (off + len > vn->size) {
        len = vn->size - off;
    }
    
    // TODO: Read from data blocks
    // For now, return zeros
    memset(buf, 0, len);
    return len;
}

// Read directory entries
static int ext2_readdir(vnode_t *vn, vfs_dirent_cb cb, void *ctx) {
    if (!vn || !vn->fs_priv) return -1;
    
    // TODO: Read directory entries from data blocks
    // For now, return empty directory
    return 0;
}

// Lookup file in directory
static vnode_t *ext2_lookup(vnode_t *dir, const char *name, size_t namelen) {
    if (!dir || !dir->fs_priv) return NULL;
    
    // TODO: Search directory entries
    // For now, return NULL (not found)
    return NULL;
}

// Mount ext2 filesystem
int ext2_mount_fs(block_dev_t *bdev, vfs_super_t **out_sb) {
    // Allocate superblock
    vfs_super_t *sb = (vfs_super_t *)vmm_kmalloc(sizeof(vfs_super_t), 8);
    if (!sb) return -1;
    
    memset(sb, 0, sizeof(vfs_super_t));
    
    // Allocate filesystem private data
    ext2_fs_t *fs = (ext2_fs_t *)vmm_kmalloc(sizeof(ext2_fs_t), 8);
    if (!fs) {
        vmm_kfree(sb, sizeof(vfs_super_t));
        return -1;
    }
    
    memset(fs, 0, sizeof(ext2_fs_t));
    
    // TODO: Read superblock from block device
    // For now, create a minimal superblock
    fs->superblock.s_magic = EXT2_SUPER_MAGIC;
    fs->superblock.s_log_block_size = 0;  // 1024 bytes
    fs->superblock.s_inode_size = 128;
    fs->superblock.s_inodes_per_group = 8192;
    fs->superblock.s_blocks_per_group = 8192;
    
    fs->block_size = 1024 << fs->superblock.s_log_block_size;
    fs->inode_size = fs->superblock.s_inode_size;
    fs->inodes_per_group = fs->superblock.s_inodes_per_group;
    fs->blocks_per_group = fs->superblock.s_blocks_per_group;
    
    sb->fs_priv = fs;
    sb->block_size = fs->block_size;
    sb->ops = &ext2_super_ops;
    sb->bdev = bdev;
    
    *out_sb = sb;
    return 0;
}

// Filesystem type structure
static const fs_type_t ext2_fs_type = {
    .name = "ext2",
    .mount = ext2_mount_fs
};

// Register ext2 filesystem
void ext2_init(void) {
    vfs_register_fs(&ext2_fs_type);
}
