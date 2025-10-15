#pragma once
#include "kernel.h"

#define MAX_FD 256

typedef struct fd_entry {
    void* obj;      // Pointer to file/socket/pipe/etc.
    u32   type;     // FD type (file, pipe, socket, etc.)
    u32   flags;    // O_RDONLY, O_WRONLY, O_RDWR, etc.
    u32   refcnt;   // Reference count
} fd_entry_t;

typedef struct fd_table {
    fd_entry_t entries[MAX_FD];
    spinlock_t lock;
} fd_table_t;

void fd_table_init(fd_table_t* table);
int  fd_alloc(fd_table_t* table, void* obj, u32 type, u32 flags);
int  fd_dup(fd_table_t* table, int oldfd);
int  fd_close(fd_table_t* table, int fd);
void* fd_get_obj(fd_table_t* table, int fd, u32* type_out);
