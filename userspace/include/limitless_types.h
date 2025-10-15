#ifndef LIMITLESS_TYPES_H
#define LIMITLESS_TYPES_H

/* Standard C types */
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* LimitlessOS common types */
typedef unsigned long size_t;
typedef long ssize_t;
typedef int pid_t;
typedef unsigned int uid_t;
typedef unsigned int gid_t;

/* Status and error types */
typedef enum {
    STATUS_SUCCESS = 0,
    STATUS_ERROR = -1,
    STATUS_INVALID_PARAM = -2,
    STATUS_OUT_OF_MEMORY = -3,
    STATUS_NOT_FOUND = -4,
    STATUS_PERMISSION_DENIED = -5,
    STATUS_TIMEOUT = -6,
    STATUS_BUSY = -7,
    STATUS_NOT_SUPPORTED = -8,
    STATUS_ALREADY_EXISTS = -9,
    STATUS_INVALID_STATE = -10
} status_t;

/* Common constants */
#ifndef NULL
#define NULL ((void*)0)
#endif

#ifndef EOF
#define EOF (-1)
#endif

/* Boolean values for older code */
#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

#endif /* LIMITLESS_TYPES_H */