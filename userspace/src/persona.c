/*
 * Persona Engine - Userspace implementation
 */

#include "persona.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#ifndef PERSONA_MAX_REGISTRY
#define PERSONA_MAX_REGISTRY 64
#endif

#ifndef PERSONA_MAX_HANDLES
#define PERSONA_MAX_HANDLES 128
#endif

typedef struct registry_entry {
    persona_id_t id;
    const persona_vtable_t* vt;
} registry_entry_t;

static struct {
    registry_entry_t regs[PERSONA_MAX_REGISTRY];
    size_t count;
    persona_id_t next_id;
} g_registry = { .count = 0, .next_id = 1 };

static bool str_ieq(const char* a, const char* b) {
    if (!a || !b) return false;
    while (*a && *b) {
        char ca = (char)tolower((unsigned char)*a);
        char cb = (char)tolower((unsigned char)*b);
        if (ca != cb) return false;
        ++a; ++b;
    }
    return *a == '\0' && *b == '\0';
}

static bool list_contains_ext(const char* const* list, const char* ext) {
    if (!list || !ext) return false;
    for (size_t i = 0; list[i]; ++i) {
        if (str_ieq(list[i], ext)) return true;
    }
    return false;
}

static bool check_magic(const persona_descriptor_t* d, const char* path) {
    if (!d || !d->magic || d->magic_len == 0 || !path) return false;
    FILE* f = fopen(path, "rb");
    if (!f) return false;
    uint8_t buf[64] = {0};
    size_t want = d->magic_len < sizeof(buf) ? d->magic_len : sizeof(buf);
    size_t n = fread(buf, 1, want, f);
    fclose(f);
    if (n < d->magic_len) return false;
    return memcmp(buf, d->magic, d->magic_len) == 0;
}

bool persona_get_extension(const char* path, char* extBuf, size_t extBufLen) {
    if (!path || !extBuf || extBufLen == 0) return false;
    const char* dot = NULL;
    const char* p = path;
    for (; *p; ++p) {
        if (*p == '.') dot = p;
        if (*p == '/' || *p == '\\') dot = NULL; /* reset on path separators */
    }
    if (!dot || *(dot+1) == '\0') return false;
    size_t len = strlen(dot);
    if (len >= extBufLen) len = extBufLen - 1;
    for (size_t i = 0; i < len; ++i) {
        char c = dot[i];
        extBuf[i] = (char)tolower((unsigned char)c);
    }
    extBuf[len] = '\0';
    return true;
}

persona_status_t persona_register(const persona_vtable_t* vt, persona_id_t* out_id) {
    if (!vt || !vt->desc.name || !vt->init || !vt->open || !vt->close) return PERSONA_ERR;
    if (g_registry.count >= PERSONA_MAX_REGISTRY) return PERSONA_NOMEM;
    persona_id_t id = g_registry.next_id++;
    g_registry.regs[g_registry.count++] = (registry_entry_t){ .id = id, .vt = vt };
    if (out_id) *out_id = id;
    return PERSONA_OK;
}

persona_status_t persona_unregister(persona_id_t id) {
    for (size_t i = 0; i < g_registry.count; ++i) {
        if (g_registry.regs[i].id == id) {
            /* compact */
            for (size_t j = i + 1; j < g_registry.count; ++j) {
                g_registry.regs[j-1] = g_registry.regs[j];
            }
            g_registry.count--;
            return PERSONA_OK;
        }
    }
    return PERSONA_NOTFOUND;
}

persona_status_t persona_resolve_for_path(const char* path, persona_id_t* out_id) {
    if (!path || !out_id) return PERSONA_ERR;

    char ext[32] = {0};
    bool hasExt = persona_get_extension(path, ext, sizeof(ext));

    /* First pass: try extension match */
    if (hasExt) {
        for (size_t i = 0; i < g_registry.count; ++i) {
            const persona_vtable_t* vt = g_registry.regs[i].vt;
            if (list_contains_ext(vt->desc.exts, ext)) {
                *out_id = g_registry.regs[i].id;
                return PERSONA_OK;
            }
        }
    }

    /* Second pass: magic signature */
    for (size_t i = 0; i < g_registry.count; ++i) {
        const persona_vtable_t* vt = g_registry.regs[i].vt;
        if (check_magic(&vt->desc, path)) {
            *out_id = g_registry.regs[i].id;
            return PERSONA_OK;
        }
    }

    return PERSONA_NOTFOUND;
}

persona_status_t persona_launch(persona_id_t id, const char* path, persona_handle_t** out_handle) {
    if (!path || !out_handle) return PERSONA_ERR;
    const persona_vtable_t* vt = NULL;
    for (size_t i = 0; i < g_registry.count; ++i) {
        if (g_registry.regs[i].id == id) { vt = g_registry.regs[i].vt; break; }
    }
    if (!vt) return PERSONA_NOTFOUND;

    persona_handle_t* h = (persona_handle_t*)calloc(1, sizeof(persona_handle_t));
    if (!h) return PERSONA_NOMEM;
    h->id = id;
    h->vt = vt;
    h->impl_ctx = NULL;

    persona_status_t st = vt->init(h);
    if (st != PERSONA_OK) { free(h); return st; }

    st = vt->open(h, path);
    if (st != PERSONA_OK) {
        if (vt->close) vt->close(h);
        free(h);
        return st;
    }

    *out_handle = h;
    return PERSONA_OK;
}

persona_status_t persona_send(persona_handle_t* handle, const void* msg, size_t len) {
    if (!handle || !handle->vt || !handle->vt->on_message) return PERSONA_NOIMPL;
    return handle->vt->on_message(handle, msg, len);
}

persona_status_t persona_close(persona_handle_t* handle) {
    if (!handle) return PERSONA_ERR;
    persona_status_t st = PERSONA_OK;
    if (handle->vt && handle->vt->close) st = handle->vt->close(handle);
    free(handle);
    return st;
}
