#pragma once
#include "kernel.h"
#include "syscall.h"
#ifdef __cplusplus
extern "C" {
#endif

/* Syscall hook framework
 * Provides registration for pre and post hooks.
 * Pre-hook may return a non-zero (negative errno style) to short-circuit execution.
 * Post-hook observes result (original or overridden) and may optionally override by setting *result_modified.
 */

#define SYSCALL_HOOK_MAX 16

typedef long (*syscall_pre_hook_t)(u64 num, u64* a0, u64* a1, u64* a2, u64* a3, u64* a4, u64* a5);
typedef void (*syscall_post_hook_t)(u64 num, long in_result, long* out_result, int* result_modified);

int syscall_hook_register_pre(syscall_pre_hook_t fn);
int syscall_hook_register_post(syscall_post_hook_t fn);

/* Stats for observability */
typedef struct syscall_hook_stats {
    u64 pre_invocations;
    u64 pre_short_circuits; /* times a pre hook aborted */
    u64 post_invocations;
    u64 post_overrides;     /* times post changed result */
} syscall_hook_stats_t;

const syscall_hook_stats_t* syscall_hook_get_stats(void);

/* Diagnostic helpers (internal; may be used by procfs) */
syscall_pre_hook_t* __syscall_get_pre_hooks(void);
syscall_post_hook_t* __syscall_get_post_hooks(void);

#ifdef __cplusplus
}
#endif
