/*
 * Userspace IPC shim (local-only, single-process for now)
 * This is a simple ring-buffer per endpoint for development and testing.
 * Replace with kernel-backed syscalls when available.
 */

#include "ipc.h"

#include <stdlib.h>
#include <string.h>

#ifndef IPC_MAX_ENDPOINTS
#define IPC_MAX_ENDPOINTS 128
#endif

#ifndef IPC_QUEUE_CAP
#define IPC_QUEUE_CAP 64
#endif

typedef struct {
    void* data;
    size_t len;
} msg_buf_t;

typedef struct {
    int active;
    ipc_endpoint_t id;
    msg_buf_t q[IPC_QUEUE_CAP];
    unsigned head;
    unsigned tail;
    unsigned count;
} endpoint_t;

static struct {
    endpoint_t eps[IPC_MAX_ENDPOINTS];
    ipc_endpoint_t next_id;
} g_ipc = { .next_id = 1 };

static endpoint_t* find_ep(ipc_endpoint_t id) {
    for (int i = 0; i < IPC_MAX_ENDPOINTS; ++i) {
        if (g_ipc.eps[i].active && g_ipc.eps[i].id == id) return &g_ipc.eps[i];
    }
    return NULL;
}

int ipc_create_endpoint(ipc_endpoint_t* out_ep) {
    if (!out_ep) return -1;
    for (int i = 0; i < IPC_MAX_ENDPOINTS; ++i) {
        if (!g_ipc.eps[i].active) {
            g_ipc.eps[i].active = 1;
            g_ipc.eps[i].id = g_ipc.next_id++;
            g_ipc.eps[i].head = g_ipc.eps[i].tail = g_ipc.eps[i].count = 0;
            *out_ep = g_ipc.eps[i].id;
            return 0;
        }
    }
    return -1;
}

int ipc_destroy_endpoint(ipc_endpoint_t ep) {
    endpoint_t* e = find_ep(ep);
    if (!e) return -1;
    // free queued messages
    while (e->count) {
        msg_buf_t* m = &e->q[e->head];
        if (m->data) free(m->data);
        e->head = (e->head + 1) % IPC_QUEUE_CAP;
        e->count--;
    }
    e->active = 0;
    e->id = 0;
    return 0;
}

int ipc_send(const ipc_msg_t* msg) {
    if (!msg) return -1;
    endpoint_t* dst = find_ep(msg->dst);
    if (!dst) return -1;
    if (dst->count >= IPC_QUEUE_CAP) return -3; // full
    msg_buf_t* slot = &dst->q[dst->tail];
    void* buf = NULL;
    if (msg->len) {
        buf = malloc(msg->len);
        if (!buf) return -4;
        memcpy(buf, msg->payload, msg->len);
    }
    slot->data = buf;
    slot->len = msg->len;
    dst->tail = (dst->tail + 1) % IPC_QUEUE_CAP;
    dst->count++;
    return 0;
}

int ipc_recv(ipc_endpoint_t ep, void* buf, size_t buf_len, size_t* out_len, uint32_t timeout_ms) {
    /* Implement timeout using select/poll */
    if (timeout_ms > 0) {
        fd_set readfds;
        struct timeval tv;
        FD_ZERO(&readfds);
        FD_SET(e->fd, &readfds);
        tv.tv_sec = timeout_ms / 1000;
        tv.tv_usec = (timeout_ms % 1000) * 1000;
        
        int result = select(e->fd + 1, &readfds, NULL, NULL, &tv);
        if (result <= 0) {
            return IPC_ERROR_TIMEOUT;
        }
    }
    endpoint_t* e = find_ep(ep);
    if (!e) return -1;
    if (!e->count) return -2; // empty
    msg_buf_t* m = &e->q[e->head];
    size_t n = m->len;
    if (buf && buf_len) {
        size_t to_copy = n < buf_len ? n : buf_len;
        if (m->data) memcpy(buf, m->data, to_copy);
    }
    if (out_len) *out_len = n;
    if (m->data) free(m->data);
    e->head = (e->head + 1) % IPC_QUEUE_CAP;
    e->count--;
    return 0;
}
