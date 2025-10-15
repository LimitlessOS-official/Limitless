#ifndef LIMITLESS_PERSONA_H
#define LIMITLESS_PERSONA_H

/*
 * Persona Engine (Userspace-side scaffold)
 * - Modular runtime system where each persona handles a specific file type/ecosystem
 * - Registry with auto-detection by extension and optional magic signature
 * - Simple launch/open lifecycle that personas can implement via a vtable
 *
 * NOTE:
 * This header defines a userspace persona engine API that does not depend on
 * kernel headers to ease adoption in existing userspace modules. Kernel
 * integration and sandboxing/IPC brokering can be added later behind the same
 * API surface.
 */

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Status codes (keep consistent with userspace components) */
typedef enum {
    PERSONA_OK = 0,
    PERSONA_ERR = -1,
    PERSONA_DENY = -2,
    PERSONA_NOIMPL = -3,
    PERSONA_NOMEM = -4,
    PERSONA_AGAIN = -5,
    PERSONA_NOTFOUND = -6,
} persona_status_t;

typedef uint64_t persona_id_t;

typedef struct {
    const char* name;            /* e.g., "posix", "win32", "macos", "python", "web" */
    const char* version;         /* e.g., "1.0.0" */
    const char* const* mime_types; /* null-terminated list of supported MIME types (optional) */
    const char* const* exts;       /* null-terminated list of supported extensions like ".elf" (optional) */
    const uint8_t* magic;        /* optional magic signature bytes for detection */
    size_t magic_len;            /* length of magic signature */
} persona_descriptor_t;

/* Forward decl for handle */
struct persona_handle;
typedef struct persona_handle persona_handle_t;

/* Persona vtable interface */
typedef persona_status_t (*persona_init_fn)(persona_handle_t* self);
typedef persona_status_t (*persona_open_fn)(persona_handle_t* self, const char* path);
typedef persona_status_t (*persona_msg_fn)(persona_handle_t* self, const void* msg, size_t len);
typedef persona_status_t (*persona_close_fn)(persona_handle_t* self);

typedef struct {
    persona_descriptor_t desc;
    persona_init_fn init;
    persona_open_fn open;
    persona_msg_fn on_message;
    persona_close_fn close;
} persona_vtable_t;

/* A launched persona instance (lightweight in this userspace scaffold) */
struct persona_handle {
    persona_id_t id;
    const persona_vtable_t* vt;  /* bound at launch */
    void* impl_ctx;              /* persona-private state */
    /* Optional caps/limits can be added once kernel integration is present */
};

/* Registry API */
persona_status_t persona_register(const persona_vtable_t* vt, persona_id_t* out_id);
persona_status_t persona_unregister(persona_id_t id);

/* Dispatch API */
persona_status_t persona_resolve_for_path(const char* path, persona_id_t* out_id);
persona_status_t persona_launch(persona_id_t id, const char* path, persona_handle_t** out_handle);
persona_status_t persona_send(persona_handle_t* handle, const void* msg, size_t len);
persona_status_t persona_close(persona_handle_t* handle);

/* Utilities */
/* Extract lowercase extension from path into extBuf (e.g., ".elf"). Returns true if found. */
bool persona_get_extension(const char* path, char* extBuf, size_t extBufLen);

#ifdef __cplusplus
}
#endif

#endif /* LIMITLESS_PERSONA_H */
