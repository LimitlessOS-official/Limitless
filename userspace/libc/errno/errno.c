#include <errno.h>
#include <stdio.h>

// Global errno variable
int errno = 0;

// Error messages
static const char *error_messages[] = {
    "Success",                              // 0
    "Operation not permitted",              // EPERM
    "No such file or directory",            // ENOENT
    "No such process",                      // ESRCH
    "Interrupted system call",              // EINTR
    "I/O error",                            // EIO
    "No such device or address",            // ENXIO
    "Argument list too long",               // E2BIG
    "Exec format error",                    // ENOEXEC
    "Bad file descriptor",                  // EBADF
    "No child processes",                   // ECHILD
    "Try again",                            // EAGAIN
    "Out of memory",                        // ENOMEM
    "Permission denied",                    // EACCES
    "Bad address",                          // EFAULT
    "Block device required",                // ENOTBLK
    "Device or resource busy",              // EBUSY
    "File exists",                          // EEXIST
    "Cross-device link",                    // EXDEV
    "No such device",                       // ENODEV
    "Not a directory",                      // ENOTDIR
    "Is a directory",                       // EISDIR
    "Invalid argument",                     // EINVAL
    "File table overflow",                  // ENFILE
    "Too many open files",                  // EMFILE
    "Not a typewriter",                     // ENOTTY
    "Text file busy",                       // ETXTBSY
    "File too large",                       // EFBIG
    "No space left on device",              // ENOSPC
    "Illegal seek",                         // ESPIPE
    "Read-only file system",                // EROFS
    "Too many links",                       // EMLINK
    "Broken pipe",                          // EPIPE
    "Math argument out of domain",          // EDOM
    "Math result not representable",        // ERANGE
};

#define MAX_ERROR_MSG (sizeof(error_messages) / sizeof(error_messages[0]))

// Get error string
char *strerror(int errnum) {
    static char unknown[32];
    
    if (errnum >= 0 && errnum < MAX_ERROR_MSG) {
        return (char *)error_messages[errnum];
    }
    
    // Format unknown error
    char *p = unknown;
    const char *prefix = "Unknown error ";
    while (*prefix) {
        *p++ = *prefix++;
    }
    
    // Convert error number to string
    if (errnum < 0) {
        *p++ = '-';
        errnum = -errnum;
    }
    
    char tmp[12];
    int i = 0;
    do {
        tmp[i++] = '0' + (errnum % 10);
        errnum /= 10;
    } while (errnum > 0);
    
    while (i > 0) {
        *p++ = tmp[--i];
    }
    *p = '\0';
    
    return unknown;
}

// Print error message
void perror(const char *s) {
    // TODO: This should write to stderr when we have file descriptors
    // For now, use printf
    if (s && *s) {
        printf("%s: %s\n", s, strerror(errno));
    } else {
        printf("%s\n", strerror(errno));
    }
}
