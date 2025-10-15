/*
 * fcntl.h - File control options
 * POSIX file control definitions
 */

#ifndef _FCNTL_H
#define _FCNTL_H

// File access modes (must match O_* in VFS)
#define O_RDONLY    0x0000  // Open for reading only
#define O_WRONLY    0x0001  // Open for writing only
#define O_RDWR      0x0002  // Open for reading and writing

// File creation flags
#define O_CREAT     0x0100  // Create file if it doesn't exist
#define O_EXCL      0x0200  // Error if O_CREAT and file exists
#define O_TRUNC     0x0400  // Truncate to zero length
#define O_APPEND    0x0800  // Append mode

// File status flags
#define O_NONBLOCK  0x1000  // Non-blocking I/O
#define O_SYNC      0x4000  // Synchronous writes
#define O_DIRECTORY 0x2000  // Fail if not directory

// File descriptor flags
#define FD_CLOEXEC  1       // Close on exec

// fcntl commands
#define F_DUPFD     0       // Duplicate file descriptor
#define F_GETFD     1       // Get file descriptor flags
#define F_SETFD     2       // Set file descriptor flags
#define F_GETFL     3       // Get file status flags
#define F_SETFL     4       // Set file status flags

// Function prototypes
int open(const char *pathname, int flags, ...);
int creat(const char *pathname, unsigned int mode);
int fcntl(int fd, int cmd, ...);

#endif // _FCNTL_H
