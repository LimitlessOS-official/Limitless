/*
 * ld_so.h - LimitlessOS Dynamic Linker Header
 * 
 * Provides dynamic linking, symbol resolution, and runtime library loading.
 */

#ifndef LIMITLESSOS_LD_SO_H
#define LIMITLESSOS_LD_SO_H

#include <kernel/types.h>
#include <kernel/elf.h>

/* dlopen flags */
#define RTLD_LAZY     0x00001
#define RTLD_NOW      0x00002
#define RTLD_GLOBAL   0x00100
#define RTLD_LOCAL    0x00000
#define RTLD_NODELETE 0x01000
#define RTLD_NOLOAD   0x00004
#define RTLD_DEEPBIND 0x00008

/* Special handle values for dlsym */
#define RTLD_DEFAULT  ((void*)0)
#define RTLD_NEXT     ((void*)-1L)

/* Dynamic linker initialization */
status_t ld_so_init(void);

/* Load executable */
status_t ld_so_load_executable(const char* path, void** entry_point);

/* Dynamic library interface (POSIX) */
void* dlopen(const char* filename, int flags);
void* dlsym(void* handle, const char* symbol);
int dlclose(void* handle);
char* dlerror(void);

/* Internal functions */
void* _dl_lookup_symbol(const char* name);
int _dl_iterate_phdr(int (*callback)(struct dl_phdr_info* info, size_t size, void* data), void* data);

#endif /* LIMITLESSOS_LD_SO_H */