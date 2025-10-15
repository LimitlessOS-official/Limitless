#include <oqs/common.h>
#include <stdlib.h>

// Default memory allocation if not set by the application
static void *default_malloc(size_t size) {
    return malloc(size);
}

static void default_free(void *ptr) {
    free(ptr);
}

// Simple cleanse function
static void default_cleanse(void *ptr, size_t len) {
    volatile uint8_t *p = ptr;
    while (len--) {
        *p++ = 0;
    }
}

void OQS_init(void) {
    if (OQS_MEM_alloc == NULL) {
        OQS_MEM_alloc = default_malloc;
    }
    if (OQS_MEM_free == NULL) {
        OQS_MEM_free = default_free;
    }
    if (OQS_MEM_cleanse == NULL) {
        OQS_MEM_cleanse = default_cleanse;
    }
}

// Dummy randombytes for kernel environment.
// In a real kernel, this would use a hardware RNG or entropy pool.
OQS_STATUS OQS_randombytes(uint8_t *random_array, size_t bytes_to_generate) {
    for (size_t i = 0; i < bytes_to_generate; i++) {
        random_array[i] = (uint8_t)rand();
    }
    return OQS_SUCCESS;
}
