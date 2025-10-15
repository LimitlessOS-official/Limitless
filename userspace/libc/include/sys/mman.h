/*
 * sys/mman.h - Memory management declarations
 * POSIX memory mapping interface
 */

#ifndef _SYS_MMAN_H
#define _SYS_MMAN_H

#include <stddef.h>

// Type definitions
typedef long off_t;

// Protection flags for mmap
#define PROT_NONE  0x0  // Pages cannot be accessed
#define PROT_READ  0x1  // Pages can be read
#define PROT_WRITE 0x2  // Pages can be written
#define PROT_EXEC  0x4  // Pages can be executed

// Mapping flags
#define MAP_SHARED    0x01  // Share changes
#define MAP_PRIVATE   0x02  // Changes are private
#define MAP_FIXED     0x10  // Interpret addr exactly
#define MAP_ANONYMOUS 0x20  // Don't use a file (anonymous mapping)
#define MAP_ANON      MAP_ANONYMOUS  // Alias for MAP_ANONYMOUS

// msync flags
#define MS_ASYNC      0x01  // Sync memory asynchronously
#define MS_SYNC       0x02  // Synchronous memory sync
#define MS_INVALIDATE 0x04  // Invalidate cached data

// Special return value for mmap errors
#define MAP_FAILED ((void *) -1)

// Function prototypes
void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
int munmap(void *addr, size_t length);
int msync(void *addr, size_t length, int flags);
int mprotect(void *addr, size_t length, int prot);
int madvise(void *addr, size_t length, int advice);

#endif // _SYS_MMAN_H
