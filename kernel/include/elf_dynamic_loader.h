#pragma once

#include "kernel.h"
#include "vmm.h"
#include "elf.h"
#include "elf_loader.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Enhanced ELF executable information */
typedef struct elf_executable_info {
    Elf64_Ehdr ehdr;
    bool is_64bit;
    bool is_executable;
    bool is_shared_object;
    bool is_dynamic;
    bool has_interpreter;
    uint64_t entry_point;
    char interpreter[256];
} elf_executable_info_t;

/* Auxiliary vector types (subset) */
#define AT_NULL     0   /* End of vector */
#define AT_IGNORE   1   /* Entry should be ignored */
#define AT_EXECFD   2   /* File descriptor of program */
#define AT_PHDR     3   /* Program headers for program */
#define AT_PHENT    4   /* Size of program header entry */
#define AT_PHNUM    5   /* Number of program headers */
#define AT_PAGESZ   6   /* System page size */
#define AT_BASE     7   /* Base address of interpreter */
#define AT_FLAGS    8   /* Flags */
#define AT_ENTRY    9   /* Entry point of program */
#define AT_NOTELF   10  /* Program is not ELF */
#define AT_UID      11  /* Real uid */
#define AT_EUID     12  /* Effective uid */
#define AT_GID      13  /* Real gid */
#define AT_EGID     14  /* Effective gid */

/* Load executable with full dynamic linking support */
int elf_load_executable(vmm_aspace_t* aspace, const char* path, const char* const argv[], 
                       const char* const envp[], elf_load_result_t* result);

/* Load all dependencies specified in DT_NEEDED entries */
int elf_load_dependencies(vmm_aspace_t* aspace, const char* main_path);

/* Setup process environment (argv, envp, auxiliary vector) */
int elf_setup_process_environment(vmm_aspace_t* aspace, const char* const argv[], 
                                 const char* const envp[], elf_load_result_t* result);

/* Get information about executable without loading it */
int elf_get_executable_info(const char* path, elf_executable_info_t* info);

#ifdef __cplusplus
}
#endif