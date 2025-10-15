// Minimal USTAR tar reader/writer helpers (reader-only for now)
#pragma once
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct tar_ctx tar_ctx_t;

typedef struct {
    const char* path;     // normalized path (forward slashes)
    uint64_t    size;     // file size
    int         type;     // '0' or '\0' = reg, '5' = dir, '2' = symlink (ignored)
    const char* linkname; // if type '2'
} tar_entry_t;

// Open a tar file for reading
tar_ctx_t* tar_open(const char* tar_path);
void       tar_close(tar_ctx_t* ctx);

// Iterate entries; returns 1 when an entry is produced, 0 on end, <0 on error
int tar_next(tar_ctx_t* ctx, tar_entry_t* out);

// Read up to len bytes for current entry's data into buf; returns number read
size_t tar_read_data(tar_ctx_t* ctx, void* buf, size_t len);

// Skip remaining data in current entry and align to next header
int tar_skip_entry(tar_ctx_t* ctx);

#ifdef __cplusplus
}
#endif
