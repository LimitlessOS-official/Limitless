#ifndef LIMITLESS_INSTALLER_STATUS_H
#define LIMITLESS_INSTALLER_STATUS_H

#ifdef __cplusplus
extern "C" {
#endif

/* Installer status codes */
typedef enum {
    STATUS_OK = 0,
    STATUS_ERROR = 1,
    STATUS_BUSY = 2,
    STATUS_INVALID = 3,
    STATUS_NOT_IMPLEMENTED = 4,
    STATUS_UNSUPPORTED = 5,
    STATUS_TIMEOUT = 6,
    STATUS_PARTIAL = 7,
    STATUS_SUCCESS = 0,
    STATUS_ALREADY_INITIALIZED = 8,
    STATUS_NOT_INITIALIZED = 9,
    STATUS_INVALID_PARAMETER = 10,
    STATUS_OUT_OF_MEMORY = 11,
    STATUS_NOT_FOUND = 12
} status_t;

#ifdef __cplusplus
}
#endif

#endif /* LIMITLESS_INSTALLER_STATUS_H */
