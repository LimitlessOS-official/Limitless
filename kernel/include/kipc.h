#pragma once
#include "kernel.h"

/*
 * Kernel IPC primitives (Phase 2)
 * - Message queues (endpoints)
 * - Shared memory handles (placeholder)
 * - Events/signals
 */

typedef u32 kendpoint_t;

typedef struct {
    kendpoint_t src;
    kendpoint_t dst;
    u32 len;
    u8  data[1024];
} kipc_msg_t;

int kipc_endpoint_create(kendpoint_t* out);
int kipc_endpoint_close(kendpoint_t ep);
int kipc_send(const kipc_msg_t* m);
int kipc_recv(kendpoint_t ep, kipc_msg_t* out, u32 timeout_ms);