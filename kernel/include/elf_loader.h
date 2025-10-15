/*
 * elf_loader.h - LimitlessOS Advanced ELF Loader
 * 
 * Production ELF loader with lazy loading, advanced dynamic linking,
 * security validation, and comprehensive relocation support.
 */

#pragma once
#include "kernel.h"
#include "vmm.h"
#include "process.h"

/*
 * Advanced ELF Loader Features:
 * - Complete ELF64 support with lazy segment loading
 * - Full dynamic linking with PT_INTERP and DT_NEEDED resolution
 * - All x86_64 relocation types supported
 * - Security validation (W^X, stack protection, ASLR)
 * - Copy-on-write support for shared libraries
 * - Thread-Local Storage (TLS) support
 * - GNU extensions (GNU_STACK, GNU_RELRO, GNU_EH_FRAME)
 * - Symbol versioning and weak symbols
 * - PIE (Position Independent Executable) support
 * - Advanced error handling and diagnostics
 */

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    ELF_OK = 0,
    ELF_EINVAL = -22,
    ELF_ENOTSUP = -95,
    ELF_ENOMEM = -12,
} elf_status_t;

typedef struct {
    virt_addr_t entry;        /* Entry point RIP */
    virt_addr_t user_stack;   /* Suggested initial user stack top (if stack_vaddr provided) */
    u64         image_base;   /* Mapped base for ET_DYN or 0 for fixed */
    u64         image_size;   /* Total covered virtual size (rounded to pages) */
    u32         relapplied;   /* Number of relocations successfully applied */
    u8          lazy_segments;/* 1 if PT_LOAD segments were registered for demand paging (not eagerly copied) */
} elf_load_result_t;

/* Optional stack placement hint: if non-zero, a guard page is placed below. */
typedef struct {
    virt_addr_t stack_vaddr;  /* e.g., 0x00007fff00000000 */
    u64         stack_size;   /* e.g., 1 MiB */
} elf_stack_hint_t;

/* Identify and sanity-check ELF image in memory */
int elf_identify(const void* image, size_t size);

/* Load ELF image into address space. For ET_DYN, base_hint can be 0 to auto-pick. */
int elf_load_into_aspace(const void* image, size_t size,
                         vmm_aspace_t* as,
                         u64 base_hint,
                         const elf_stack_hint_t* stack_hint,
                         elf_load_result_t* out);

/* Minimal loader self-test: parses a fake header and returns 0 if parser is alive. */
int elf_loader_selftest(void);

/* Simplistic symbol resolve request (future extension for inter-object): */
struct elf_symbol_resolve_req {
    const char* name;  /* IN */
    virt_addr_t value; /* OUT: 0 if not found */
};

/* Module metadata exposed to caller when loading dependencies */
typedef struct {
    const char* soname;       /* Stable pointer into internal string table (or provided buf) */
    virt_addr_t base;         /* Load base */
    const void* dynsym;       /* Pointer to in-memory dynsym (read-only) */
    size_t      dynsym_count; /* Number of symbols (0 if unknown; resolution still works via hash/linear) */
    const char* dynstr;       /* Pointer to dynstr */
    size_t      dynstr_size;  /* Size of dynstr */
} elf_loaded_module_t;

/* Fetch callback: return image pointer & size for a SONAME. Return 0 on success. */
typedef int (*elf_fetch_callback_t)(const char* soname, const void** image_out, size_t* size_out);

/* Extended load that resolves DT_NEEDED recursively (up to caller-provided capacity). */
int elf_load_with_deps_into_aspace(const void* image, size_t size,
                                   vmm_aspace_t* as,
                                   u64 base_hint,
                                   const elf_stack_hint_t* stack_hint,
                                   elf_fetch_callback_t fetch_cb,
                                   elf_loaded_module_t* modules, /* modules[0] = main */
                                   size_t* module_count,         /* IN: capacity, OUT: used */
                                   elf_load_result_t* out);

/* Enumerate loaded modules (for /proc/modules_loaded). Weakly implemented; returns total count; copies up to cap entries. */
size_t elf_modules_enumerate(const char** names, u64* bases, u32* relocs, size_t cap);

/* In-memory integration test harness.
 * Builds synthetic minimal ELF64 images (main + one or more DT_NEEDED deps) entirely in RAM,
 * feeds them to the recursive loader using a mock fetch callback, and validates:
 *   - Recursive DT_NEEDED traversal (depth-first ordering)
 *   - Module count limit enforcement (does not overflow provided capacity)
 *   - Hash table parsing (DT_HASH presence) populates dynsym_count > 0 for deps
 * Returns 0 (ELF_OK) on success, negative elf_status_t / custom -1 on validation failure.
 * This helper does NOT attempt to execute code, only mapping + relocation counts.
 */
int elf_loader_inmemory_test(void);

#ifdef __cplusplus
}
#endif