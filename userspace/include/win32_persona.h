/* win32_persona.h - minimized stub; full Win32 layer disabled */
#ifndef LIMITLESS_WIN32_PERSONA_H
#define LIMITLESS_WIN32_PERSONA_H

#include <stdint.h>

typedef struct { int unused; } win32_context_t;
typedef struct { uint64_t syscalls_translated; } win32_stats_t;

static inline int win32_persona_init(void){ return 0; }
static inline int win32_persona_shutdown(void){ return 0; }
static inline win32_context_t* win32_get_context(void){ return 0; }
static inline int win32_get_stats(win32_stats_t* out_stats){ if(out_stats) out_stats->syscalls_translated=0; return 0; }

#endif /* LIMITLESS_WIN32_PERSONA_H */
