#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "persona.h"
#include "ipc.h"

/*
 * Service Manager (Phase 3)
 * - Simple dependency-based startup
 * - Each service is represented by a persona-bound "unit"
 * - Crash-restart is simulated by relaunching persona upon stop request
 */

#ifdef __cplusplus
extern "C" {
#endif

#define SM_MAX_SERVICES 32
#define SM_MAX_DEPS 8

typedef enum {
    SM_STOPPED = 0,
    SM_STARTING,
    SM_RUNNING,
    SM_FAILED,
} sm_state_t;

typedef struct {
    const char* name;
    const char* path;             /* Target file path, used with persona resolver */
    const char* deps[SM_MAX_DEPS];/* Null-terminated list of dependency names */
    int restart_on_crash;         /* Not used in Phase 3 (no crash detection) */
} service_desc_t;

typedef struct service {
    service_desc_t desc;
    sm_state_t state;
    persona_handle_t* handle;   /* active persona instance */
} service_t;

void sm_init(void);

/* Register an array of service descriptors (NULL name terminates) */
int sm_register(const service_desc_t* list);

/* Start all services honoring dependencies (naive ordering) */
int sm_start_all(void);

/* Dump status to stdout */
void sm_status_dump(void);

/* Stop all (best-effort) */
void sm_stop_all(void);

#ifdef __cplusplus
}
#endif