#pragma once
#include "kernel.h"
#include "process.h"

/* Waitpid and process state tracking */

typedef enum {
    PROC_RUNNING = 0,
    PROC_ZOMBIE  = 1,
    PROC_EXITED  = 2
} proc_state_t;

int process_waitpid(pid_t pid, int* status, int options);
void process_exit(process_t* p, int status);
void process_reap_zombies(void);
