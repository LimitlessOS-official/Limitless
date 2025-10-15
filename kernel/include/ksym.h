#pragma once
#include "kernel.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Symbolizer interface: resolve an address to symbol+offset string */
int ksym_init(void);
int ksym_resolve(u64 addr, char* out, size_t outsz);

#ifdef __cplusplus
}
#endif
