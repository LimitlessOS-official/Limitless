/*
 * TCP (Transmission Control Protocol) Implementation
 * 
 * Production-grade TCP/IP stack matching Linux/BSD quality.
 * Full RFC 793 compliance with modern extensions (RFC 1323, 2018, 5681, etc.)
 * 
 * Features:
 * - Full TCP state machine (LISTEN, SYN-SENT, ESTABLISHED, etc.)
 * - Reliable data transfer with retransmission
 * - Flow control (sliding window)
 * - Congestion control (Reno, NewReno, CUBIC)
 * - Fast retransmit and fast recovery
 * - Selective acknowledgments (SACK)
 * - Window scaling
 * - Timestamp support
 * - Keep-alive
 * - Nagle's algorithm
 * - SYN cookies (SYN flood protection)
 * 
 * Copyright (c) 2025 LimitlessOS Project
 */

#ifndef NET_TCP_FULL_H
#define NET_TCP_FULL_H

#include <stdint.h>
#include "net/skbuff.h"
#include "net/ip.h"

#ifdef __cplusplus
extern "C" {
#endif

/* TCP states (RFC 793) */
typedef enum {
    TCP_CLOSED       = 0,
    TCP_LISTEN       = 1,
    TCP_SYN_SENT     = 2,
    TCP_SYN_RECV     = 3,
    TCP_ESTABLISHED  = 4,
    TCP_FIN_WAIT1    = 5,
    TCP_FIN_WAIT2    = 6,
    TCP_CLOSE_WAIT   = 7,
    TCP_CLOSING      = 8,
    TCP_LAST_ACK     = 9,
    TCP_TIME_WAIT    = 10,
    TCP_MAX_STATES
} tcp_state_t;

/* TCP header (RFC 793) */
typedef struct tcphdr {
    uint16_t source;        /* Source port */
    uint16_t dest;          /* Destination port */
    uint32_t seq;           /* Sequence number */
    uint32_t ack_seq;       /* Acknowledgment number */
    uint16_t res1:4,        /* Reserved */
             doff:4,        /* Data offset (header length in 32-bit words) */
             fin:1,         /* FIN flag */
             syn:1,         /* SYN flag */
             rst:1,         /* RST flag */
             psh:1,         /* PSH flag */
             ack:1,         /* ACK flag */
             urg:1,         /* URG flag */
             ece:1,         /* ECE flag (ECN) */
             cwr:1;         /* CWR flag (ECN) */
    uint16_t window;        /* Window size */
    uint16_t check;         /* Checksum */
    uint16_t urg_ptr;       /* Urgent pointer */
    /* Options may follow */
} __attribute__((packed)) tcphdr_t;

/* TCP flags */
#define TCP_FLAG_FIN  0x01
#define TCP_FLAG_SYN  0x02
#define TCP_FLAG_RST  0x04
#define TCP_FLAG_PSH  0x08
#define TCP_FLAG_ACK  0x10
#define TCP_FLAG_URG  0x20
#define TCP_FLAG_ECE  0x40
#define TCP_FLAG_CWR  0x80

/* TCP options */
#define TCPOPT_EOL           0  /* End of option list */
#define TCPOPT_NOP           1  /* No operation */
#define TCPOPT_MAXSEG        2  /* Maximum segment size */
#define TCPOPT_WINDOW        3  /* Window scale */
#define TCPOPT_SACK_PERM     4  /* SACK permitted */
#define TCPOPT_SACK          5  /* SACK */
#define TCPOPT_TIMESTAMP     8  /* Timestamps */

#define TCPOLEN_MAXSEG       4
#define TCPOLEN_WINDOW       3
#define TCPOLEN_SACK_PERM    2
#define TCPOLEN_TIMESTAMP    10

/* TCP constants */
#define TCP_MSS_DEFAULT      536    /* Default MSS */
#define TCP_MSS_DESIRED      1460   /* Desired MSS */
#define TCP_MAX_WINDOW       65535  /* Maximum window size */
#define TCP_INITIAL_WINDOW   10     /* Initial congestion window (segments) */
#define TCP_MIN_RTO          200    /* Minimum RTO (ms) */
#define TCP_MAX_RTO          120000 /* Maximum RTO (ms) */
#define TCP_RTO_INITIAL      3000   /* Initial RTO (ms) */
#define TCP_DELACK_MIN       40     /* Minimum delayed ACK timeout (ms) */
#define TCP_DELACK_MAX       200    /* Maximum delayed ACK timeout (ms) */
#define TCP_KEEPALIVE_TIME   7200000/* Keep-alive time (2 hours in ms) */
#define TCP_KEEPALIVE_INTVL  75000  /* Keep-alive interval (75s in ms) */
#define TCP_KEEPALIVE_PROBES 9      /* Keep-alive probes */
#define TCP_MAX_RETRIES      15     /* Maximum retransmission attempts */
#define TCP_SYN_RETRIES      6      /* SYN retransmission attempts */
#define TCP_TIMEWAIT_LEN     60000  /* TIME-WAIT duration (60s in ms) */

/* Congestion control algorithms */
typedef enum {
    TCP_CA_RENO = 0,
    TCP_CA_NEWRENO,
    TCP_CA_CUBIC,
    TCP_CA_BBR
} tcp_ca_algorithm_t;

/* TCP congestion control state */
typedef struct tcp_ca_state {
    tcp_ca_algorithm_t algorithm;
    uint32_t cwnd;          /* Congestion window */
    uint32_t ssthresh;      /* Slow start threshold */
    uint32_t bytes_acked;   /* Bytes ACKed this RTT */
    uint32_t prior_cwnd;    /* Prior cwnd for recovery */
    uint8_t ca_state;       /* CA state (open, disorder, CWR, recovery, loss) */
    uint8_t retransmits;    /* Retransmission count */
    
    /* CUBIC specific */
    struct {
        uint32_t epoch_start;
        uint32_t k;
        uint32_t origin_point;
        uint32_t w_max;
        uint32_t w_tcp;
        uint32_t last_time;
    } cubic;
    
    /* BBR specific */
    struct {
        uint32_t min_rtt;
        uint32_t max_bw;
        uint8_t mode;
        uint8_t phase;
    } bbr;
} tcp_ca_state_t;

/* TCP retransmission queue entry */
typedef struct tcp_retrans_queue {
    sk_buff_t* skb;                /* Packet to retransmit */
    uint32_t seq;                  /* Sequence number */
    uint64_t timestamp;            /* Time sent */
    uint8_t retries;               /* Retransmission count */
    struct tcp_retrans_queue* next;
} tcp_retrans_queue_t;

/* TCP receive buffer entry */
typedef struct tcp_recv_buf {
    uint8_t* data;          /* Data buffer */
    uint32_t len;           /* Data length */
    uint32_t seq;           /* Sequence number */
    struct tcp_recv_buf* next;
} tcp_recv_buf_t;

/* TCP socket structure */
typedef struct tcp_sock {
    /* Connection state */
    tcp_state_t state;
    
    /* Addressing */
    ipv4_addr_t local_addr;
    ipv4_addr_t remote_addr;
    uint16_t local_port;
    uint16_t remote_port;
    
    /* Sequence numbers */
    uint32_t snd_una;       /* Send unacknowledged */
    uint32_t snd_nxt;       /* Send next */
    uint32_t snd_wnd;       /* Send window */
    uint32_t snd_wl1;       /* Segment sequence number for last window update */
    uint32_t snd_wl2;       /* Segment acknowledgment number for last window update */
    uint32_t iss;           /* Initial send sequence number */
    
    uint32_t rcv_nxt;       /* Receive next */
    uint32_t rcv_wnd;       /* Receive window */
    uint32_t irs;           /* Initial receive sequence number */
    
    /* Timers */
    uint64_t retrans_timer;    /* Retransmission timer */
    uint64_t delack_timer;     /* Delayed ACK timer */
    uint64_t keepalive_timer;  /* Keep-alive timer */
    uint64_t timewait_timer;   /* TIME-WAIT timer */
    
    /* RTT estimation (RFC 6298) */
    uint32_t srtt;          /* Smoothed RTT */
    uint32_t rttvar;        /* RTT variance */
    uint32_t rto;           /* Retransmission timeout */
    uint32_t mdev_max;      /* Maximum RTT deviation */
    
    /* Window management */
    uint16_t mss;           /* Maximum segment size */
    uint8_t window_scale;   /* Window scale factor */
    uint8_t snd_wscale;     /* Send window scale */
    uint8_t rcv_wscale;     /* Receive window scale */
    
    /* Options */
    uint8_t timestamps_ok;  /* Timestamps enabled */
    uint8_t sack_ok;        /* SACK enabled */
    uint32_t ts_recent;     /* Recent timestamp */
    uint32_t ts_recent_age; /* Age of ts_recent */
    
    /* Congestion control */
    tcp_ca_state_t ca;
    
    /* Buffers */
    sk_buff_head_t write_queue;      /* Send queue */
    tcp_retrans_queue_t* retrans_queue; /* Retransmission queue */
    tcp_recv_buf_t* recv_buf;        /* Out-of-order receive buffer */
    sk_buff_head_t ofo_queue;        /* Out-of-order queue */
    
    /* Buffer sizes */
    uint32_t sndbuf;        /* Send buffer size */
    uint32_t rcvbuf;        /* Receive buffer size */
    
    /* Flags */
    uint32_t nonagle:1,     /* Nagle's algorithm disabled */
             quickack:1,    /* Quick ACK mode */
             cork:1,        /* Cork mode (TCP_CORK) */
             defer_accept:1,/* Defer accept */
             keepalive:1,   /* Keep-alive enabled */
             linger:1,      /* Linger on close */
             reuse_addr:1,  /* SO_REUSEADDR */
             reuse_port:1,  /* SO_REUSEPORT */
             broadcast:1,   /* Broadcast allowed */
             oobinline:1;   /* OOB data inline */
    
    /* Statistics */
    uint64_t segments_in;   /* Segments received */
    uint64_t segments_out;  /* Segments sent */
    uint64_t bytes_in;      /* Bytes received */
    uint64_t bytes_out;     /* Bytes sent */
    uint32_t retransmits;   /* Retransmissions */
    
    /* Listen queue (for listening sockets) */
    struct {
        struct tcp_sock** queue;  /* Backlog queue */
        uint32_t qlen;            /* Current queue length */
        uint32_t max_qlen;        /* Maximum queue length */
    } listen;
    
    /* Back-pointer to parent socket structure */
    void* sock;
    
    /* Hash table linkage */
    struct tcp_sock* hash_next;
    struct tcp_sock* hash_prev;
    
} tcp_sock_t;

/* TCP statistics */
typedef struct tcp_stats {
    uint64_t active_opens;          /* Active connections opened */
    uint64_t passive_opens;         /* Passive connections opened */
    uint64_t attempt_fails;         /* Failed connection attempts */
    uint64_t estab_resets;          /* Connections reset in ESTABLISHED */
    uint64_t curr_estab;            /* Current ESTABLISHED connections */
    uint64_t in_segs;               /* Segments received */
    uint64_t out_segs;              /* Segments sent */
    uint64_t retrans_segs;          /* Segments retransmitted */
    uint64_t in_errs;               /* Bad segments received */
    uint64_t out_rsts;              /* RST segments sent */
    uint64_t in_csum_errors;        /* Checksum errors */
} tcp_stats_t;

/* ==================== Core TCP Functions ==================== */

/* Socket operations */
tcp_sock_t* tcp_socket_create(void);
void tcp_socket_destroy(tcp_sock_t* sk);
int tcp_bind(tcp_sock_t* sk, ipv4_addr_t addr, uint16_t port);
int tcp_listen(tcp_sock_t* sk, int backlog);
int tcp_connect(tcp_sock_t* sk, ipv4_addr_t addr, uint16_t port);
tcp_sock_t* tcp_accept(tcp_sock_t* sk);
int tcp_close(tcp_sock_t* sk);

/* Data transfer */
int tcp_send(tcp_sock_t* sk, const void* data, size_t len, int flags);
int tcp_recv(tcp_sock_t* sk, void* data, size_t len, int flags);
int tcp_sendmsg(tcp_sock_t* sk, sk_buff_t* skb);
int tcp_recvmsg(tcp_sock_t* sk, void* buf, size_t len);

/* Packet processing */
int tcp_rcv(sk_buff_t* skb);
int tcp_process(tcp_sock_t* sk, sk_buff_t* skb);
int tcp_input(sk_buff_t* skb);
int tcp_output(tcp_sock_t* sk);

/* State machine */
int tcp_handle_syn(tcp_sock_t* sk, sk_buff_t* skb);
int tcp_handle_established(tcp_sock_t* sk, sk_buff_t* skb);
int tcp_handle_close(tcp_sock_t* sk);
void tcp_set_state(tcp_sock_t* sk, tcp_state_t state);

/* Transmission */
int tcp_transmit_skb(tcp_sock_t* sk, sk_buff_t* skb, int cloned);
int tcp_write_xmit(tcp_sock_t* sk, unsigned int mss_now);
int tcp_push(tcp_sock_t* sk, int flags);
void tcp_send_syn(tcp_sock_t* sk);
void tcp_send_synack(tcp_sock_t* sk);
void tcp_send_ack(tcp_sock_t* sk);
void tcp_send_fin(tcp_sock_t* sk);
void tcp_send_reset(tcp_sock_t* sk, sk_buff_t* skb);

/* Reception */
void tcp_data_queue(tcp_sock_t* sk, sk_buff_t* skb);
void tcp_ack(tcp_sock_t* sk, sk_buff_t* skb);
void tcp_fast_path_on(tcp_sock_t* sk);
int tcp_prequeue(tcp_sock_t* sk, sk_buff_t* skb);

/* Retransmission */
void tcp_retransmit_timer(tcp_sock_t* sk);
int tcp_retransmit_skb(tcp_sock_t* sk, sk_buff_t* skb);
void tcp_clear_retrans(tcp_sock_t* sk);
void tcp_update_rto(tcp_sock_t* sk, uint32_t rtt_sample);

/* Congestion control */
void tcp_ca_init(tcp_sock_t* sk, tcp_ca_algorithm_t algorithm);
void tcp_ca_on_ack(tcp_sock_t* sk, uint32_t acked);
void tcp_ca_on_loss(tcp_sock_t* sk);
void tcp_ca_on_retrans(tcp_sock_t* sk);
void tcp_slow_start(tcp_sock_t* sk);
void tcp_congestion_avoidance(tcp_sock_t* sk);
void tcp_enter_recovery(tcp_sock_t* sk);
void tcp_leave_recovery(tcp_sock_t* sk);

/* Fast retransmit/recovery */
void tcp_enter_fast_recovery(tcp_sock_t* sk);
void tcp_fastretrans_alert(tcp_sock_t* sk);
int tcp_sack_process(tcp_sock_t* sk, sk_buff_t* skb);

/* Timers */
void tcp_timer_tick(void);
void tcp_keepalive_timer(tcp_sock_t* sk);
void tcp_delack_timer(tcp_sock_t* sk);
void tcp_timewait_timer(tcp_sock_t* sk);

/* Socket lookup */
tcp_sock_t* tcp_lookup(ipv4_addr_t saddr, uint16_t sport, ipv4_addr_t daddr, uint16_t dport);
tcp_sock_t* tcp_lookup_listen(ipv4_addr_t daddr, uint16_t dport);
void tcp_hash(tcp_sock_t* sk);
void tcp_unhash(tcp_sock_t* sk);

/* Options */
void tcp_parse_options(sk_buff_t* skb, tcp_sock_t* sk);
int tcp_build_options(tcp_sock_t* sk, uint8_t* ptr, int length);
void tcp_select_initial_window(tcp_sock_t* sk);

/* Checksum */
uint16_t tcp_checksum(const void* tcphdr, size_t len, ipv4_addr_t saddr, ipv4_addr_t daddr);
uint16_t tcp_v4_check(const tcphdr_t* th, int len, ipv4_addr_t saddr, ipv4_addr_t daddr, uint32_t base);

/* Utilities */
const char* tcp_state_str(tcp_state_t state);
void tcp_dump_sock(const tcp_sock_t* sk);
void tcp_get_stats(tcp_stats_t* stats);
void tcp_reset_stats(void);

/* Initialization */
int tcp_init(void);
void tcp_cleanup(void);

/* Helper functions */
static inline tcphdr_t* tcp_hdr(const sk_buff_t* skb) {
    return (tcphdr_t*)skb_transport_header(skb);
}

static inline int tcp_hdrlen(const sk_buff_t* skb) {
    return tcp_hdr(skb)->doff * 4;
}

static inline int tcp_optlen(const sk_buff_t* skb) {
    return tcp_hdrlen(skb) - sizeof(tcphdr_t);
}

static inline uint8_t* tcp_options(const sk_buff_t* skb) {
    return (uint8_t*)tcp_hdr(skb) + sizeof(tcphdr_t);
}

#ifdef __cplusplus
}
#endif

#endif /* NET_TCP_FULL_H */
