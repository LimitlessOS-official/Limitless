/* Networking & IPC Hook Framework */
#pragma once
#include "kernel.h"
#include "kipc.h"

#ifdef __cplusplus
extern "C" { 
#endif

#define NET_IPC_HOOK_MAX 16

/* Packet direction */
typedef enum { HOOK_DIR_RX=0, HOOK_DIR_TX=1 } hook_direction_t;

/* Simplified packet meta (IPv4 focus) */
typedef struct net_packet_meta {
    hook_direction_t dir; /* RX or TX */
    u16 proto;            /* L4: 6 TCP, 17 UDP, others */
    u32 src_ip_be;
    u32 dst_ip_be;
    u16 src_port_be;
    u16 dst_port_be;
    u32 length;           /* payload length (excluding IP hdr) */
    const void* data;     /* pointer to packet payload (RO) */
    int allow_mutation;   /* if nonzero hook may write via cast */
    int verdict;          /* 0 continue, <0 drop or error code */
} net_packet_meta_t;

typedef void (*net_packet_hook_t)(net_packet_meta_t* m);

/* IPC message hook: called on send and on delivery (recv). */
typedef enum { IPC_STAGE_SEND=0, IPC_STAGE_RECV=1 } ipc_stage_t;
typedef struct ipc_msg_meta {
    ipc_stage_t stage;
    const kipc_msg_t* msg; /* original pointer */
    int verdict;           /* pre-send hooks can set negative to abort */
} ipc_msg_meta_t;
typedef void (*ipc_msg_hook_t)(ipc_msg_meta_t* meta);

int net_register_packet_hook(net_packet_hook_t fn);
int ipc_register_msg_hook(ipc_msg_hook_t fn);

/* Stats */
typedef struct net_ipc_hook_stats {
    u64 pkt_rx_invoked;
    u64 pkt_tx_invoked;
    u64 pkt_dropped;      /* count of hooks setting verdict<0 */
    u64 pkt_hooks;        /* registration count */
    u64 ipc_send_invoked;
    u64 ipc_recv_invoked;
    u64 ipc_send_blocked;
    u64 ipc_hooks;
} net_ipc_hook_stats_t;

const net_ipc_hook_stats_t* net_ipc_get_stats(void);
void net_ipc_reset_stats(void);

/* Internal emission helpers used by stack */
void __net_emit_packet_rx(u32 src,u32 dst,u16 proto,u16 sport,u16 dport,const void* data,u32 len);
void __net_emit_packet_tx(u32 src,u32 dst,u16 proto,u16 sport,u16 dport,const void* data,u32 len);
/* Future TCP emission convenience wrappers (no-op if not used yet) */
static inline void net_hook_emit_tcp_tx(u32 src,u32 dst,u16 sport,u16 dport,const void* data,u32 len){ __net_emit_packet_tx(src,dst,6,sport,dport,data,len); }
static inline void net_hook_emit_tcp_rx(u32 src,u32 dst,u16 sport,u16 dport,const void* data,u32 len){ __net_emit_packet_rx(src,dst,6,sport,dport,data,len); }
int  __ipc_emit_send(const kipc_msg_t* m); /* returns 0 or error if blocked */
void __ipc_emit_recv(const kipc_msg_t* m);

#ifdef __cplusplus
}
#endif