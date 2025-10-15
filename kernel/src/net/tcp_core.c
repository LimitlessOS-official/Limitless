/*
 * TCP Implementation - Part 1: Core State Machine and Connection Management
 * 
 * Production-grade TCP implementation with full state machine.
 * This file implements the core connection management logic.
 * 
 * Copyright (c) 2025 LimitlessOS Project
 */

#include "net/tcp_full.h"
#include "net/ip.h"
#include "kernel.h"
#include <string.h>

/* Global TCP state */
static struct {
    tcp_sock_t* listen_hash[256];   /* Listening sockets hash table */
    tcp_sock_t* conn_hash[1024];    /* Connected sockets hash table */
    uint32_t isn_secret;            /* ISN generation secret */
    tcp_stats_t stats;              /* Global statistics */
    uint32_t lock;                  /* Global lock */
} tcp_state;

/* State name strings for debugging */
static const char* tcp_state_names[TCP_MAX_STATES] = {
    "CLOSED",
    "LISTEN",
    "SYN_SENT",
    "SYN_RECV",
    "ESTABLISHED",
    "FIN_WAIT1",
    "FIN_WAIT2",
    "CLOSE_WAIT",
    "CLOSING",
    "LAST_ACK",
    "TIME_WAIT"
};

const char* tcp_state_str(tcp_state_t state) {
    if (state >= TCP_MAX_STATES) {
        return "UNKNOWN";
    }
    return tcp_state_names[state];
}

/* ==================== Socket Lifecycle ==================== */

tcp_sock_t* tcp_socket_create(void) {
    tcp_sock_t* sk = (tcp_sock_t*)kmalloc(sizeof(tcp_sock_t));
    if (!sk) {
        kprintf("[TCP] Failed to allocate socket\n");
        return NULL;
    }
    
    memset(sk, 0, sizeof(tcp_sock_t));
    
    /* Initialize state */
    sk->state = TCP_CLOSED;
    
    /* Initialize queues */
    skb_queue_head_init(&sk->write_queue);
    skb_queue_head_init(&sk->ofo_queue);
    
    /* Set defaults */
    sk->mss = TCP_MSS_DEFAULT;
    sk->snd_wnd = TCP_MAX_WINDOW;
    sk->rcv_wnd = TCP_MAX_WINDOW;
    sk->sndbuf = 65536;  /* 64KB send buffer */
    sk->rcvbuf = 65536;  /* 64KB receive buffer */
    sk->rto = TCP_RTO_INITIAL;
    
    /* Initialize congestion control */
    tcp_ca_init(sk, TCP_CA_CUBIC);
    
    kprintf("[TCP] Created socket %p\n", sk);
    
    return sk;
}

void tcp_socket_destroy(tcp_sock_t* sk) {
    if (!sk) return;
    
    kprintf("[TCP] Destroying socket %p state=%s\n", sk, tcp_state_str(sk->state));
    
    /* Remove from hash tables */
    tcp_unhash(sk);
    
    /* Free queues */
    skb_queue_purge(&sk->write_queue);
    skb_queue_purge(&sk->ofo_queue);
    
    /* Free retransmission queue */
    tcp_clear_retrans(sk);
    
    /* Free receive buffer */
    tcp_recv_buf_t* buf = sk->recv_buf;
    while (buf) {
        tcp_recv_buf_t* next = buf->next;
        if (buf->data) kfree(buf->data);
        kfree(buf);
        buf = next;
    }
    
    /* Free listen queue if present */
    if (sk->listen.queue) {
        kfree(sk->listen.queue);
    }
    
    kfree(sk);
}

/* ==================== Socket Operations ==================== */

int tcp_bind(tcp_sock_t* sk, ipv4_addr_t addr, uint16_t port) {
    if (!sk) return -1;
    
    if (sk->state != TCP_CLOSED) {
        kprintf("[TCP] Cannot bind socket in state %s\n", tcp_state_str(sk->state));
        return -1;
    }
    
    /* Check if port is already in use */
    tcp_sock_t* existing = tcp_lookup_listen(addr, port);
    if (existing && !sk->reuse_addr) {
        kprintf("[TCP] Port %u already in use\n", port);
        return -1;
    }
    
    sk->local_addr = addr;
    sk->local_port = port;
    
    kprintf("[TCP] Bound socket to %s:%u\n", 
            ip_addr_to_str(addr, NULL, 0), port);
    
    return 0;
}

int tcp_listen(tcp_sock_t* sk, int backlog) {
    if (!sk) return -1;
    
    if (sk->state != TCP_CLOSED) {
        kprintf("[TCP] Cannot listen on socket in state %s\n", tcp_state_str(sk->state));
        return -1;
    }
    
    /* Allocate listen queue */
    if (backlog <= 0) backlog = 5;
    if (backlog > 128) backlog = 128;
    
    sk->listen.queue = (tcp_sock_t**)kmalloc(sizeof(tcp_sock_t*) * backlog);
    if (!sk->listen.queue) {
        kprintf("[TCP] Failed to allocate listen queue\n");
        return -1;
    }
    
    memset(sk->listen.queue, 0, sizeof(tcp_sock_t*) * backlog);
    sk->listen.max_qlen = backlog;
    sk->listen.qlen = 0;
    
    /* Move to LISTEN state */
    tcp_set_state(sk, TCP_LISTEN);
    
    /* Add to listen hash table */
    tcp_hash(sk);
    
    kprintf("[TCP] Socket listening on %s:%u (backlog=%d)\n",
            ip_addr_to_str(sk->local_addr, NULL, 0),
            sk->local_port,
            backlog);
    
    tcp_state.stats.passive_opens++;
    
    return 0;
}

int tcp_connect(tcp_sock_t* sk, ipv4_addr_t addr, uint16_t port) {
    if (!sk) return -1;
    
    if (sk->state != TCP_CLOSED) {
        kprintf("[TCP] Cannot connect socket in state %s\n", tcp_state_str(sk->state));
        return -1;
    }
    
    /* Set remote address */
    sk->remote_addr = addr;
    sk->remote_port = port;
    
    /* If local address not set, pick one */
    if (sk->local_addr == 0) {
        /* TODO: Select source address based on routing */
        sk->local_addr = ip_make_addr(192, 168, 1, 100);  /* Placeholder */
    }
    
    /* If local port not set, pick ephemeral port */
    if (sk->local_port == 0) {
        /* TODO: Proper ephemeral port allocation */
        static uint16_t next_port = 32768;
        sk->local_port = next_port++;
        if (next_port > 61000) next_port = 32768;
    }
    
    /* Generate initial sequence number */
    sk->iss = tcp_generate_isn(sk);
    sk->snd_una = sk->iss;
    sk->snd_nxt = sk->iss + 1;
    
    /* Move to SYN_SENT state */
    tcp_set_state(sk, TCP_SYN_SENT);
    
    /* Add to hash table */
    tcp_hash(sk);
    
    /* Send SYN packet */
    tcp_send_syn(sk);
    
    kprintf("[TCP] Connecting to %s:%u from %s:%u\n",
            ip_addr_to_str(addr, NULL, 0), port,
            ip_addr_to_str(sk->local_addr, NULL, 0), sk->local_port);
    
    tcp_state.stats.active_opens++;
    
    return 0;
}

tcp_sock_t* tcp_accept(tcp_sock_t* sk) {
    if (!sk) return NULL;
    
    if (sk->state != TCP_LISTEN) {
        kprintf("[TCP] Cannot accept on socket in state %s\n", tcp_state_str(sk->state));
        return NULL;
    }
    
    /* Check listen queue */
    if (sk->listen.qlen == 0) {
        /* No pending connections */
        return NULL;
    }
    
    /* Get first connection from queue */
    tcp_sock_t* new_sk = sk->listen.queue[0];
    
    /* Shift queue */
    for (uint32_t i = 0; i < sk->listen.qlen - 1; i++) {
        sk->listen.queue[i] = sk->listen.queue[i + 1];
    }
    sk->listen.queue[sk->listen.qlen - 1] = NULL;
    sk->listen.qlen--;
    
    kprintf("[TCP] Accepted connection from %s:%u\n",
            ip_addr_to_str(new_sk->remote_addr, NULL, 0),
            new_sk->remote_port);
    
    return new_sk;
}

int tcp_close(tcp_sock_t* sk) {
    if (!sk) return -1;
    
    kprintf("[TCP] Closing socket in state %s\n", tcp_state_str(sk->state));
    
    switch (sk->state) {
        case TCP_CLOSED:
            /* Already closed */
            tcp_socket_destroy(sk);
            return 0;
            
        case TCP_LISTEN:
            /* Just close listening socket */
            tcp_set_state(sk, TCP_CLOSED);
            tcp_socket_destroy(sk);
            return 0;
            
        case TCP_SYN_SENT:
            /* Connection never established */
            tcp_set_state(sk, TCP_CLOSED);
            tcp_socket_destroy(sk);
            return 0;
            
        case TCP_SYN_RECV:
        case TCP_ESTABLISHED:
            /* Send FIN and wait */
            tcp_send_fin(sk);
            tcp_set_state(sk, TCP_FIN_WAIT1);
            return 0;
            
        case TCP_CLOSE_WAIT:
            /* Send FIN */
            tcp_send_fin(sk);
            tcp_set_state(sk, TCP_LAST_ACK);
            return 0;
            
        case TCP_FIN_WAIT1:
        case TCP_FIN_WAIT2:
        case TCP_CLOSING:
        case TCP_LAST_ACK:
        case TCP_TIME_WAIT:
            /* Already closing */
            return 0;
    }
    
    return 0;
}

/* ==================== State Management ==================== */

void tcp_set_state(tcp_sock_t* sk, tcp_state_t new_state) {
    tcp_state_t old_state = sk->state;
    
    if (old_state == new_state) {
        return;
    }
    
    kprintf("[TCP] State transition: %s -> %s\n",
            tcp_state_str(old_state),
            tcp_state_str(new_state));
    
    sk->state = new_state;
    
    /* Update statistics */
    if (new_state == TCP_ESTABLISHED && old_state != TCP_ESTABLISHED) {
        tcp_state.stats.curr_estab++;
    } else if (old_state == TCP_ESTABLISHED && new_state != TCP_ESTABLISHED) {
        tcp_state.stats.curr_estab--;
        
        if (new_state == TCP_CLOSED) {
            tcp_state.stats.estab_resets++;
        }
    }
    
    /* Start TIME-WAIT timer if needed */
    if (new_state == TCP_TIME_WAIT) {
        sk->timewait_timer = get_ticks() + (TCP_TIMEWAIT_LEN / 10);  /* Convert ms to ticks */
    }
}

/* ==================== Hash Table Management ==================== */

static uint32_t tcp_hash_func(ipv4_addr_t addr, uint16_t port) {
    return (addr ^ port) & 0x3FF;  /* 1024 buckets */
}

void tcp_hash(tcp_sock_t* sk) {
    if (!sk) return;
    
    if (sk->state == TCP_LISTEN) {
        /* Add to listen hash */
        uint32_t hash = tcp_hash_func(sk->local_addr, sk->local_port) & 0xFF;
        sk->hash_next = tcp_state.listen_hash[hash];
        if (tcp_state.listen_hash[hash]) {
            tcp_state.listen_hash[hash]->hash_prev = sk;
        }
        tcp_state.listen_hash[hash] = sk;
        sk->hash_prev = NULL;
    } else {
        /* Add to connection hash */
        uint32_t hash = tcp_hash_func(sk->local_addr ^ sk->remote_addr,
                                      sk->local_port ^ sk->remote_port);
        sk->hash_next = tcp_state.conn_hash[hash];
        if (tcp_state.conn_hash[hash]) {
            tcp_state.conn_hash[hash]->hash_prev = sk;
        }
        tcp_state.conn_hash[hash] = sk;
        sk->hash_prev = NULL;
    }
}

void tcp_unhash(tcp_sock_t* sk) {
    if (!sk) return;
    
    if (sk->hash_prev) {
        sk->hash_prev->hash_next = sk->hash_next;
    } else {
        /* Head of list */
        if (sk->state == TCP_LISTEN) {
            uint32_t hash = tcp_hash_func(sk->local_addr, sk->local_port) & 0xFF;
            if (tcp_state.listen_hash[hash] == sk) {
                tcp_state.listen_hash[hash] = sk->hash_next;
            }
        } else {
            uint32_t hash = tcp_hash_func(sk->local_addr ^ sk->remote_addr,
                                          sk->local_port ^ sk->remote_port);
            if (tcp_state.conn_hash[hash] == sk) {
                tcp_state.conn_hash[hash] = sk->hash_next;
            }
        }
    }
    
    if (sk->hash_next) {
        sk->hash_next->hash_prev = sk->hash_prev;
    }
    
    sk->hash_next = NULL;
    sk->hash_prev = NULL;
}

tcp_sock_t* tcp_lookup(ipv4_addr_t saddr, uint16_t sport, ipv4_addr_t daddr, uint16_t dport) {
    uint32_t hash = tcp_hash_func(daddr ^ saddr, dport ^ sport);
    tcp_sock_t* sk = tcp_state.conn_hash[hash];
    
    while (sk) {
        if (sk->local_addr == daddr &&
            sk->local_port == dport &&
            sk->remote_addr == saddr &&
            sk->remote_port == sport) {
            return sk;
        }
        sk = sk->hash_next;
    }
    
    return NULL;
}

tcp_sock_t* tcp_lookup_listen(ipv4_addr_t daddr, uint16_t dport) {
    uint32_t hash = tcp_hash_func(daddr, dport) & 0xFF;
    tcp_sock_t* sk = tcp_state.listen_hash[hash];
    
    while (sk) {
        if (sk->local_port == dport &&
            (sk->local_addr == 0 || sk->local_addr == daddr)) {
            return sk;
        }
        sk = sk->hash_next;
    }
    
    return NULL;
}

/* ==================== Utilities ==================== */

uint32_t tcp_generate_isn(tcp_sock_t* sk) {
    /* Simple ISN generation - should be more secure in production */
    static uint32_t isn_counter = 0;
    uint32_t isn = (get_ticks() << 16) ^ tcp_state.isn_secret ^ 
                   sk->local_addr ^ sk->remote_addr ^
                   (sk->local_port << 16) ^ sk->remote_port ^
                   (isn_counter++);
    return isn;
}

void tcp_dump_sock(const tcp_sock_t* sk) {
    if (!sk) {
        kprintf("[TCP] NULL socket\n");
        return;
    }
    
    char local_str[32], remote_str[32];
    ip_addr_to_str(sk->local_addr, local_str, sizeof(local_str));
    ip_addr_to_str(sk->remote_addr, remote_str, sizeof(remote_str));
    
    kprintf("[TCP] Socket %p:\n", sk);
    kprintf("  State: %s\n", tcp_state_str(sk->state));
    kprintf("  Local: %s:%u\n", local_str, sk->local_port);
    kprintf("  Remote: %s:%u\n", remote_str, sk->remote_port);
    kprintf("  SND: una=%u nxt=%u wnd=%u\n", sk->snd_una, sk->snd_nxt, sk->snd_wnd);
    kprintf("  RCV: nxt=%u wnd=%u\n", sk->rcv_nxt, sk->rcv_wnd);
    kprintf("  MSS: %u RTO: %u ms\n", sk->mss, sk->rto);
    kprintf("  CA: algorithm=%d cwnd=%u ssthresh=%u\n",
            sk->ca.algorithm, sk->ca.cwnd, sk->ca.ssthresh);
    kprintf("  Stats: in=%llu out=%llu retrans=%u\n",
            sk->segments_in, sk->segments_out, sk->retransmits);
}

void tcp_get_stats(tcp_stats_t* stats) {
    if (stats) {
        memcpy(stats, &tcp_state.stats, sizeof(tcp_stats_t));
    }
}

void tcp_reset_stats(void) {
    memset(&tcp_state.stats, 0, sizeof(tcp_stats_t));
}

/* ==================== Initialization ==================== */

int tcp_init(void) {
    kprintf("[TCP] Initializing TCP protocol...\n");
    
    /* Initialize hash tables */
    memset(tcp_state.listen_hash, 0, sizeof(tcp_state.listen_hash));
    memset(tcp_state.conn_hash, 0, sizeof(tcp_state.conn_hash));
    
    /* Generate ISN secret */
    tcp_state.isn_secret = get_ticks() ^ 0xDEADBEEF;  /* Simple randomization */
    
    /* Reset statistics */
    memset(&tcp_state.stats, 0, sizeof(tcp_stats_t));
    
    kprintf("[TCP] TCP protocol initialized\n");
    kprintf("[TCP] Hash tables: listen=256, conn=1024\n");
    
    return 0;
}

void tcp_cleanup(void) {
    kprintf("[TCP] Cleaning up TCP protocol...\n");
    
    /* Clean up all listening sockets */
    for (int i = 0; i < 256; i++) {
        tcp_sock_t* sk = tcp_state.listen_hash[i];
        while (sk) {
            tcp_sock_t* next = sk->hash_next;
            tcp_socket_destroy(sk);
            sk = next;
        }
    }
    
    /* Clean up all connected sockets */
    for (int i = 0; i < 1024; i++) {
        tcp_sock_t* sk = tcp_state.conn_hash[i];
        while (sk) {
            tcp_sock_t* next = sk->hash_next;
            tcp_socket_destroy(sk);
            sk = next;
        }
    }
    
    kprintf("[TCP] Statistics:\n");
    kprintf("  Active opens: %llu\n", tcp_state.stats.active_opens);
    kprintf("  Passive opens: %llu\n", tcp_state.stats.passive_opens);
    kprintf("  Segments in: %llu\n", tcp_state.stats.in_segs);
    kprintf("  Segments out: %llu\n", tcp_state.stats.out_segs);
    kprintf("  Retransmits: %llu\n", tcp_state.stats.retrans_segs);
    
    kprintf("[TCP] TCP protocol cleaned up\n");
}
