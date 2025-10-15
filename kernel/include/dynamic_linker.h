#pragma once

#include "kernel.h"
#include "vmm.h"
#include "elf.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Dynamic linker public interface */

/* Initialize dynamic linker for an address space */
int dynlink_init(vmm_aspace_t* aspace);

/* Load shared library and return base address */
int dynlink_load_library(const char* path, uint64_t* base_addr);

/* Unload shared library */
int dynlink_unload_library(const char* name);

/* Resolve symbol by name - returns address or 0 if not found */
uint64_t dynlink_resolve_symbol(const char* name);

/* Get statistics */
void dynlink_get_stats(uint32_t* lib_count, uint32_t* symbol_count);

/* ELF dynamic loader integration */
struct shared_library;
typedef struct shared_library shared_library_t;

/* Create PLT/GOT tables for lazy binding */
int dynlink_create_plt_got(shared_library_t* lib, const Elf64_Rela* relocations, size_t reloc_count);

/* Apply relocations with PLT/GOT support */
int dynlink_apply_relocations(shared_library_t* lib, const Elf64_Rela* relocations, size_t count);

/* Run library constructors */
int dynlink_run_constructors(shared_library_t* lib);

/* Get library handle by name */
shared_library_t* dynlink_get_library(const char* name);

#ifdef __cplusplus
}
#endif