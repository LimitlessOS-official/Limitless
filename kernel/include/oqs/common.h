#ifndef OQS_COMMON_H
#define OQS_COMMON_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// OQS status codes
typedef enum {
    OQS_SUCCESS = 0,
    OQS_ERROR = 1,
    OQS_EXTERNAL_LIB_ERROR_OPENSSL = 2,
} OQS_STATUS;

// OQS memory allocation callbacks
void *(*OQS_MEM_alloc)(size_t size) = NULL;
void (*OQS_MEM_free)(void *ptr) = NULL;
void (*OQS_MEM_cleanse)(void *ptr, size_t len) = NULL;

// OQS logging levels
typedef enum {
    OQS_LOG_LEVEL_DISABLED,
    OQS_LOG_LEVEL_ERROR,
    OQS_LOG_LEVEL_INFO,
    OQS_LOG_LEVEL_DEBUG,
} oqs_log_level;

// Function to initialize OQS memory allocation and logging
void OQS_init(void);

// Function to set the log level
void OQS_set_log_level(oqs_log_level level);

// Randomness generation
OQS_STATUS OQS_randombytes(uint8_t *random_array, size_t bytes_to_generate);

#endif // OQS_COMMON_H
