/*
 * File Descriptor Table
 * 
 * Manages per-process file descriptors with:
 * - open/close/read/write/lseek operations
 * - File descriptor allocation
 * - Standard streams (stdin, stdout, stderr)
 * 
 * Copyright (c) 2024 LimitlessOS Project
 */

#include "kernel.h"
#include "vfs.h"
#include <string.h>
#include <mm/mm.h>

#define MAX_FILES_PER_PROCESS 256
#define MAX_OPEN_FILES 1024

// File descriptor flags
#define FD_FLAG_OPEN    0x01
#define FD_FLAG_CLOEXEC 0x02

// Per-process file descriptor table
typedef struct {
    file_t *files[MAX_FILES_PER_PROCESS];
    u32 fd_flags[MAX_FILES_PER_PROCESS];
} fd_table_t;

// Global file table (for preallocated file_t structures)
static file_t file_table[MAX_OPEN_FILES];
static int file_table_used[MAX_OPEN_FILES];
static fd_table_t *current_fd_table = NULL;

// Standard file descriptors
#define STDIN_FILENO  0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

// Initialize file descriptor system
void fd_init(void) {
    memset(file_table, 0, sizeof(file_table));
}

// Create a new fd table for a process
fd_table_t *fd_table_create(void) {
    fd_table_t *table = (fd_table_t *)pmm_alloc_page();
    if (!table) return NULL;
    
    memset(table, 0, sizeof(fd_table_t));
    return table;
}

// Set current fd table
void fd_table_set_current(fd_table_t *table) {
    current_fd_table = table;
}

// Allocate a file structure
static file_t *alloc_file(void) {
    for (int i = 0; i < MAX_OPEN_FILES; i++) {
        if (file_table_used[i] == 0) {
            memset(&file_table[i], 0, sizeof(file_t));
            file_table_used[i] = 1;
            return &file_table[i];
        }
    }
    return NULL;
}

// Free a file structure
static void free_file(file_t *file) {
    if (!file) return;
    
    // Find the file in the table
    for (int i = 0; i < MAX_OPEN_FILES; i++) {
        if (&file_table[i] == file) {
            file_table_used[i] = 0;
            memset(file, 0, sizeof(file_t));
            return;
        }
    }
}

// Allocate a file descriptor
static int alloc_fd(fd_table_t *table) {
    if (!table) table = current_fd_table;
    if (!table) return -1;
    
    for (int i = 0; i < MAX_FILES_PER_PROCESS; i++) {
        if (table->files[i] == NULL) {
            return i;
        }
    }
    return -1;
}

// sys_open - open a file
int sys_open(const char *pathname, int flags, u32 mode) {
    if (!current_fd_table) return -1;
    
    // Allocate fd
    int fd = alloc_fd(current_fd_table);
    if (fd < 0) return -1;
    
    // Open file through VFS (vfs_open now returns file_t*)
    file_t *file;
    if (vfs_open(pathname, flags, &file) != 0) {
        return -1;
    }
    
    // Install in fd table
    current_fd_table->files[fd] = file;
    current_fd_table->fd_flags[fd] = FD_FLAG_OPEN;
    
    return fd;
}

// sys_close - close a file descriptor
int sys_close(int fd) {
    if (!current_fd_table) return -1;
    if (fd < 0 || fd >= MAX_FILES_PER_PROCESS) return -1;
    
    file_t *file = current_fd_table->files[fd];
    if (!file) return -1;
    
    // Close file through VFS
    vfs_close(file);
    
    // Clear fd table entry
    current_fd_table->files[fd] = NULL;
    current_fd_table->fd_flags[fd] = 0;
    
    return 0;
}

// sys_read - read from file descriptor
long sys_read(int fd, void *buf, size_t count) {
    if (!current_fd_table) return -1;
    if (fd < 0 || fd >= MAX_FILES_PER_PROCESS) return -1;
    
    file_t *file = current_fd_table->files[fd];
    if (!file) return -1;
    
    u64 bytes_read = 0;
    int ret = vfs_read(file, buf, count, &bytes_read);
    
    if (ret == 0 && bytes_read > 0) {
        file->offset += bytes_read;
        return (long)bytes_read;
    }
    
    return ret < 0 ? ret : 0;
}

// sys_write - write to file descriptor
long sys_write(int fd, const void *buf, size_t count) {
    if (!current_fd_table) return -1;
    if (fd < 0 || fd >= MAX_FILES_PER_PROCESS) return -1;
    
    file_t *file = current_fd_table->files[fd];
    if (!file) return -1;
    
    u64 bytes_written = 0;
    int ret = vfs_write(file, buf, count, &bytes_written);
    
    if (ret == 0 && bytes_written > 0) {
        file->offset += bytes_written;
        return (long)bytes_written;
    }
    
    return ret < 0 ? ret : 0;
}

// sys_lseek - reposition file offset
long sys_lseek(int fd, long offset, int whence) {
    if (!current_fd_table) return -1;
    if (fd < 0 || fd >= MAX_FILES_PER_PROCESS) return -1;
    
    file_t *file = current_fd_table->files[fd];
    if (!file || !file->vn) return -1;
    
    u64 new_offset;
    
    switch (whence) {
        case 0: // SEEK_SET
            new_offset = offset;
            break;
        case 1: // SEEK_CUR
            new_offset = file->offset + offset;
            break;
        case 2: // SEEK_END
            new_offset = file->vn->size + offset;
            break;
        default:
            return -1;
    }
    
    // Check bounds
    if (new_offset < 0) return -1;
    
    file->offset = new_offset;
    return new_offset;
}

// sys_fstat - get file status
// TODO: Implement when vfs_stat_t is defined in vfs.h
/*
int sys_fstat(int fd, vfs_stat_t *statbuf) {
    if (!current_fd_table) return -1;
    if (fd < 0 || fd >= MAX_FILES_PER_PROCESS) return -1;
    
    file_t *file = current_fd_table->files[fd];
    if (!file || !file->vnode) return -1;
    
    // Fill stat buffer
    statbuf->st_ino = file->vnode->ino;
    statbuf->st_size = file->vnode->size;
    statbuf->st_mode = file->vnode->mode;
    
    return 0;
}
*/

// sys_dup - duplicate file descriptor
int sys_dup(int oldfd) {
    if (!current_fd_table) return -1;
    if (oldfd < 0 || oldfd >= MAX_FILES_PER_PROCESS) return -1;
    
    file_t *file = current_fd_table->files[oldfd];
    if (!file) return -1;
    
    // Allocate new fd
    int newfd = alloc_fd(current_fd_table);
    if (newfd < 0) return -1;
    
    // Share file structure (both fds point to same file_t)
    current_fd_table->files[newfd] = file;
    current_fd_table->fd_flags[newfd] = FD_FLAG_OPEN;
    
    return newfd;
}

// sys_dup2 - duplicate to specific fd
int sys_dup2(int oldfd, int newfd) {
    if (!current_fd_table) return -1;
    if (oldfd < 0 || oldfd >= MAX_FILES_PER_PROCESS) return -1;
    if (newfd < 0 || newfd >= MAX_FILES_PER_PROCESS) return -1;
    
    if (oldfd == newfd) return newfd;
    
    file_t *file = current_fd_table->files[oldfd];
    if (!file) return -1;
    
    // Close newfd if open
    if (current_fd_table->files[newfd]) {
        sys_close(newfd);
    }
    
    // Share file structure (both fds point to same file_t)
    current_fd_table->files[newfd] = file;
    current_fd_table->fd_flags[newfd] = FD_FLAG_OPEN;
    
    return newfd;
}

// Initialize standard streams
void fd_init_stdio(fd_table_t *table) {
    // TODO: Open /dev/console for stdin/stdout/stderr
    // For now, just mark as open
    if (!table) return;
    
    // Allocate dummy files for stdio
    for (int i = 0; i < 3; i++) {
        file_t *file = alloc_file();
        if (file) {
            table->files[i] = file;
            table->fd_flags[i] = FD_FLAG_OPEN;
        }
    }
}
