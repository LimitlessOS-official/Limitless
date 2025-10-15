/* macos_persona.h - minimized stub (feature disabled) */
#ifndef LIMITLESS_MACOS_PERSONA_H
#define LIMITLESS_MACOS_PERSONA_H

#include <stdint.h>

typedef struct macos_persona_context { int unused; } macos_persona_context_t;

static inline void* macos_persona_get_context(void){ return 0; }
int macos_persona_init(void);
int macos_persona_shutdown(void);

#endif /* LIMITLESS_MACOS_PERSONA_H */
