#ifndef LIMITLESS_USER_IPC_H
#define LIMITLESS_USER_IPC_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint64_t ipc_endpoint_t; // capability-like ID

// IPC message descriptor for API calls
typedef struct {
    ipc_endpoint_t src;
    ipc_endpoint_t dst;
    const void* payload; // send: pointer to data to send; recv: ignored
    size_t len;          // length of payload
    uint32_t flags;      // reserved for future use
} ipc_msg_t;

// Create an endpoint (returns 0 on success)
int ipc_create_endpoint(ipc_endpoint_t* out_ep);

// Destroy an endpoint
int ipc_destroy_endpoint(ipc_endpoint_t ep);

// Send a message to dst endpoint (copying payload)
int ipc_send(const ipc_msg_t* msg);

// Receive a message for ep; copies into buf (up to buf_len). out_len set to actual size.
// timeout_ms=0 is non-blocking; this implementation never blocks and returns -2 on timeout/empty.
int ipc_recv(ipc_endpoint_t ep, void* buf, size_t buf_len, size_t* out_len, uint32_t timeout_ms);

#ifdef __cplusplus
}
#endif

#endif /* LIMITLESS_USER_IPC_H */
