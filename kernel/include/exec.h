#pragma once
#include "kernel.h"
#include "vmm.h"
#include "elf_loader.h"

/*
 * Exec subsystem
 * - Load ELF from VFS and spawn user process with initial thread
 * - Provide helper to start PID 1 (/sbin/init)
 */

#ifdef __cplusplus
extern "C" {
#endif

typedef struct process process_t;
typedef struct thread thread_t;

/* Spawn a user process by path (in VFS). argv/envp are arrays of pointers in kernel space. */
int exec_spawn(const char* path, const char* const argv[], const char* const envp[], u32* out_pid);

/* Internal: load an ELF image from VFS path into new aspace, return entry+stack */
int exec_load_from_path(const char* path, vmm_aspace_t* as, elf_load_result_t* out_res);

/* Kernel boot path: create PID 1 from "/sbin/init" (or fallback) */
int exec_start_pid1(const char* init_path);

#ifdef __cplusplus
}
#endif