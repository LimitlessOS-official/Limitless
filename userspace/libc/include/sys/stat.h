/*
 * sys/stat.h - File status
 * POSIX stat structure and functions
 */

#ifndef _SYS_STAT_H
#define _SYS_STAT_H

#include <stdint.h>

// File types (st_mode bits)
#define S_IFMT   0170000  // File type mask
#define S_IFREG  0100000  // Regular file
#define S_IFDIR  0040000  // Directory
#define S_IFCHR  0020000  // Character device
#define S_IFBLK  0060000  // Block device
#define S_IFIFO  0010000  // FIFO/pipe
#define S_IFLNK  0120000  // Symbolic link
#define S_IFSOCK 0140000  // Socket

// File mode bits (permissions)
#define S_ISUID  0004000  // Set user ID on execution
#define S_ISGID  0002000  // Set group ID on execution
#define S_ISVTX  0001000  // Sticky bit

#define S_IRUSR  0000400  // Read by owner
#define S_IWUSR  0000200  // Write by owner
#define S_IXUSR  0000100  // Execute by owner
#define S_IRGRP  0000040  // Read by group
#define S_IWGRP  0000020  // Write by group
#define S_IXGRP  0000010  // Execute by group
#define S_IROTH  0000004  // Read by others
#define S_IWOTH  0000002  // Write by others
#define S_IXOTH  0000001  // Execute by others

// Convenience macros
#define S_IRWXU  (S_IRUSR | S_IWUSR | S_IXUSR)
#define S_IRWXG  (S_IRGRP | S_IWGRP | S_IXGRP)
#define S_IRWXO  (S_IROTH | S_IWOTH | S_IXOTH)

// Type test macros
#define S_ISREG(m)  (((m) & S_IFMT) == S_IFREG)
#define S_ISDIR(m)  (((m) & S_IFMT) == S_IFDIR)
#define S_ISCHR(m)  (((m) & S_IFMT) == S_IFCHR)
#define S_ISBLK(m)  (((m) & S_IFMT) == S_IFBLK)
#define S_ISFIFO(m) (((m) & S_IFMT) == S_IFIFO)
#define S_ISLNK(m)  (((m) & S_IFMT) == S_IFLNK)
#define S_ISSOCK(m) (((m) & S_IFMT) == S_IFSOCK)

// stat structure
struct stat {
    uint32_t st_dev;      // Device ID
    uint32_t st_ino;      // Inode number
    uint16_t st_mode;     // File type and mode
    uint16_t st_nlink;    // Number of hard links
    uint16_t st_uid;      // User ID
    uint16_t st_gid;      // Group ID
    uint32_t st_rdev;     // Device ID (if special file)
    uint64_t st_size;     // File size in bytes
    uint64_t st_atime;    // Last access time
    uint64_t st_mtime;    // Last modification time
    uint64_t st_ctime;    // Last status change time
    uint32_t st_blksize;  // Block size for I/O
    uint32_t st_blocks;   // Number of 512B blocks
};

// Function prototypes
int stat(const char *path, struct stat *buf);
int fstat(int fd, struct stat *buf);
int lstat(const char *path, struct stat *buf);
int mkdir(const char *path, uint16_t mode);
int chmod(const char *path, uint16_t mode);

#endif // _SYS_STAT_H
