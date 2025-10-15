/*
 * TCP Implementation - Part 3: Congestion Control and Timers
 * 
 * This file implements TCP congestion control algorithms (Reno, NewReno, CUBIC, BBR)
 * and timer management (retransmission, delayed ACK, keepalive, TIME-WAIT).
 * 
 * Copyright (c) 2025 LimitlessOS Project
 */

#include "net/tcp_full.h"
#include "kernel.h"
#include <string.h>

/* ==================== Congestion Control Algorithms ==================== */

void tcp_ca_init(tcp_sock_t* sk, tcp_ca_algorithm_t algo) {
    if (!sk) return;
    
    sk->ca.algorithm = algo;
    sk->ca.cwnd = TCP_INIT_CWND * sk->mss;  /* Initial window: 10 MSS */
    sk->ca.ssthresh = TCP_MAX_WINDOW;        /* Initial threshold */
    sk->ca.state = TCP_CA_OPEN;
    
    /* Algorithm-specific initialization */
    switch (algo) {
        case TCP_CA_CUBIC:
            sk->ca.cubic.last_cwnd = 0;
            sk->ca.cubic.last_time = 0;
            sk->ca.cubic.epoch_start = 0;
            sk->ca.cubic.origin_point = 0;
            sk->ca.cubic.tcp_friendliness = 1;
            break;
            
        case TCP_CA_BBR:
            sk->ca.bbr.pacing_rate = 0;
            sk->ca.bbr.min_rtt = UINT32_MAX;
            sk->ca.bbr.probe_rtt_done_stamp = 0;
            sk->ca.bbr.mode = 0;  /* STARTUP */
            sk->ca.bbr.round_start = 0;
            sk->ca.bbr.lt_bw = 0;
            break;
            
        default:
            /* Reno/NewReno don't need special init */
            break;
    }
    
    kprintf("[TCP CA] Initialized %s algorithm: cwnd=%u ssthresh=%u\n",
            tcp_ca_name(algo), sk->ca.cwnd, sk->ca.ssthresh);
}

const char* tcp_ca_name(tcp_ca_algorithm_t algo) {
    switch (algo) {
        case TCP_CA_RENO: return "Reno";
        case TCP_CA_NEWRENO: return "NewReno";
        case TCP_CA_CUBIC: return "CUBIC";
        case TCP_CA_BBR: return "BBR";
        default: return "Unknown";
    }
}

void tcp_ca_on_ack(tcp_sock_t* sk, uint32_t acked_bytes) {
    if (!sk || acked_bytes == 0) return;
    
    /* Dispatch to algorithm-specific handler */
    switch (sk->ca.algorithm) {
        case TCP_CA_RENO:
            tcp_ca_reno_on_ack(sk, acked_bytes);
            break;
            
        case TCP_CA_NEWRENO:
            tcp_ca_newreno_on_ack(sk, acked_bytes);
            break;
            
        case TCP_CA_CUBIC:
            tcp_ca_cubic_on_ack(sk, acked_bytes);
            break;
            
        case TCP_CA_BBR:
            tcp_ca_bbr_on_ack(sk, acked_bytes);
            break;
    }
}

void tcp_ca_on_loss(tcp_sock_t* sk) {
    if (!sk) return;
    
    kprintf("[TCP CA] Packet loss detected\n");
    
    /* Common loss handling */
    sk->ca.state = TCP_CA_LOSS;
    
    /* Dispatch to algorithm */
    switch (sk->ca.algorithm) {
        case TCP_CA_RENO:
        case TCP_CA_NEWRENO:
            tcp_ca_reno_on_loss(sk);
            break;
            
        case TCP_CA_CUBIC:
            tcp_ca_cubic_on_loss(sk);
            break;
            
        case TCP_CA_BBR:
            tcp_ca_bbr_on_loss(sk);
            break;
    }
}

void tcp_ca_on_data_sent(tcp_sock_t* sk, uint32_t bytes) {
    if (!sk) return;
    
    /* BBR uses this for pacing */
    if (sk->ca.algorithm == TCP_CA_BBR) {
        tcp_ca_bbr_on_data_sent(sk, bytes);
    }
}

/* ==================== TCP Reno Congestion Control ==================== */

void tcp_ca_reno_on_ack(tcp_sock_t* sk, uint32_t acked_bytes) {
    if (sk->ca.state == TCP_CA_OPEN) {
        /* Slow start or congestion avoidance */
        if (sk->ca.cwnd < sk->ca.ssthresh) {
            /* Slow start: exponential growth */
            sk->ca.cwnd += acked_bytes;
            kprintf("[TCP CA Reno] Slow start: cwnd=%u\n", sk->ca.cwnd);
        } else {
            /* Congestion avoidance: linear growth */
            uint32_t increase = (acked_bytes * sk->mss) / sk->ca.cwnd;
            sk->ca.cwnd += increase;
            kprintf("[TCP CA Reno] Congestion avoidance: cwnd=%u\n", sk->ca.cwnd);
        }
    } else if (sk->ca.state == TCP_CA_RECOVERY) {
        /* Fast recovery */
        sk->ca.cwnd += sk->mss;
        kprintf("[TCP CA Reno] Fast recovery: cwnd=%u\n", sk->ca.cwnd);
    }
}

void tcp_ca_reno_on_loss(tcp_sock_t* sk) {
    /* Multiplicative decrease */
    sk->ca.ssthresh = sk->ca.cwnd / 2;
    if (sk->ca.ssthresh < 2 * sk->mss) {
        sk->ca.ssthresh = 2 * sk->mss;
    }
    
    /* Fast retransmit/recovery */
    sk->ca.cwnd = sk->ca.ssthresh + 3 * sk->mss;
    sk->ca.state = TCP_CA_RECOVERY;
    
    kprintf("[TCP CA Reno] Loss: ssthresh=%u cwnd=%u\n",
            sk->ca.ssthresh, sk->ca.cwnd);
}

/* ==================== TCP NewReno Congestion Control ==================== */

void tcp_ca_newreno_on_ack(tcp_sock_t* sk, uint32_t acked_bytes) {
    /* NewReno is similar to Reno but handles partial ACKs differently */
    tcp_ca_reno_on_ack(sk, acked_bytes);
    
    /* Exit recovery when all retransmitted data is acked */
    if (sk->ca.state == TCP_CA_RECOVERY && sk->snd_una >= sk->ca.recover) {
        sk->ca.state = TCP_CA_OPEN;
        sk->ca.cwnd = sk->ca.ssthresh;
        kprintf("[TCP CA NewReno] Exiting recovery: cwnd=%u\n", sk->ca.cwnd);
    }
}

/* ==================== TCP CUBIC Congestion Control ==================== */

void tcp_ca_cubic_on_ack(tcp_sock_t* sk, uint32_t acked_bytes) {
    if (sk->ca.state != TCP_CA_OPEN) {
        /* In recovery, behave like Reno */
        tcp_ca_reno_on_ack(sk, acked_bytes);
        return;
    }
    
    uint32_t now = get_ticks();
    
    /* Initialize epoch on first ACK after loss */
    if (sk->ca.cubic.epoch_start == 0) {
        sk->ca.cubic.epoch_start = now;
        
        if (sk->ca.cwnd < sk->ca.cubic.last_cwnd) {
            /* Fast convergence */
            sk->ca.cubic.last_cwnd = sk->ca.cwnd;
            sk->ca.cubic.origin_point = sk->ca.cubic.last_cwnd;
        } else {
            sk->ca.cubic.origin_point = sk->ca.cubic.last_cwnd;
        }
    }
    
    /* Calculate time since epoch (in ms) */
    uint32_t t = (now - sk->ca.cubic.epoch_start) * 10;  /* ticks to ms */
    
    /* CUBIC function: W(t) = C(t - K)^3 + W_max */
    /* Simplified calculation */
    uint32_t K = 100;  /* Estimate - should calculate based on parameters */
    int32_t dt = (int32_t)t - (int32_t)K;
    uint32_t target;
    
    if (dt > 0) {
        /* Concave region */
        uint32_t cube = (dt / 10) * (dt / 10) * (dt / 10);  /* Approximate */
        target = sk->ca.cubic.origin_point + cube;
    } else {
        /* Convex region */
        uint32_t cube = (-dt / 10) * (-dt / 10) * (-dt / 10);
        target = sk->ca.cubic.origin_point - cube;
    }
    
    /* TCP friendliness */
    if (sk->ca.cubic.tcp_friendliness) {
        uint32_t tcp_cwnd = sk->ca.cubic.last_cwnd + (acked_bytes * sk->mss) / sk->ca.cwnd;
        if (tcp_cwnd > target) {
            target = tcp_cwnd;
        }
    }
    
    /* Update window */
    if (target > sk->ca.cwnd) {
        uint32_t increase = (target - sk->ca.cwnd) / 100;  /* Gradual increase */
        if (increase < sk->mss) increase = sk->mss;
        sk->ca.cwnd += increase;
    }
    
    kprintf("[TCP CA CUBIC] t=%u target=%u cwnd=%u\n", t, target, sk->ca.cwnd);
}

void tcp_ca_cubic_on_loss(tcp_sock_t* sk) {
    /* Save current window as last max */
    sk->ca.cubic.last_cwnd = sk->ca.cwnd;
    sk->ca.cubic.last_time = get_ticks();
    sk->ca.cubic.epoch_start = 0;  /* Reset epoch */
    
    /* Multiplicative decrease (beta = 0.7 for CUBIC) */
    sk->ca.ssthresh = (sk->ca.cwnd * 7) / 10;
    if (sk->ca.ssthresh < 2 * sk->mss) {
        sk->ca.ssthresh = 2 * sk->mss;
    }
    
    sk->ca.cwnd = sk->ca.ssthresh;
    sk->ca.state = TCP_CA_RECOVERY;
    
    kprintf("[TCP CA CUBIC] Loss: last_cwnd=%u ssthresh=%u cwnd=%u\n",
            sk->ca.cubic.last_cwnd, sk->ca.ssthresh, sk->ca.cwnd);
}

/* ==================== TCP BBR Congestion Control ==================== */

void tcp_ca_bbr_on_ack(tcp_sock_t* sk, uint32_t acked_bytes) {
    uint32_t now = get_ticks();
    
    /* Update RTT measurements */
    if (sk->srtt < sk->ca.bbr.min_rtt) {
        sk->ca.bbr.min_rtt = sk->srtt;
    }
    
    /* Estimate bandwidth */
    uint32_t delivered = acked_bytes;
    uint32_t interval = now - sk->ca.bbr.round_start;
    if (interval > 0) {
        uint32_t bw = (delivered * 1000) / interval;  /* bytes/sec */
        
        /* Update max bandwidth */
        if (bw > sk->ca.bbr.lt_bw) {
            sk->ca.bbr.lt_bw = bw;
        }
    }
    
    /* Calculate pacing rate: rate = bandwidth * pacing_gain */
    uint32_t pacing_gain = 100;  /* Start with 1.0x */
    
    switch (sk->ca.bbr.mode) {
        case 0:  /* STARTUP */
            pacing_gain = 289;  /* 2/ln(2) ≈ 2.89 */
            if (sk->ca.cwnd >= sk->ca.ssthresh) {
                sk->ca.bbr.mode = 1;  /* DRAIN */
            }
            break;
            
        case 1:  /* DRAIN */
            pacing_gain = 100;
            if (sk->ca.cwnd <= sk->ca.ssthresh) {
                sk->ca.bbr.mode = 2;  /* PROBE_BW */
            }
            break;
            
        case 2:  /* PROBE_BW */
            pacing_gain = 125;  /* 1.25x */
            break;
            
        case 3:  /* PROBE_RTT */
            pacing_gain = 100;
            if (now - sk->ca.bbr.probe_rtt_done_stamp > 200) {
                sk->ca.bbr.mode = 2;  /* Back to PROBE_BW */
            }
            break;
    }
    
    sk->ca.bbr.pacing_rate = (sk->ca.bbr.lt_bw * pacing_gain) / 100;
    
    /* Set cwnd based on BDP */
    uint32_t bdp = (sk->ca.bbr.lt_bw * sk->ca.bbr.min_rtt) / 1000;
    sk->ca.cwnd = bdp * 2;  /* 2x BDP */
    
    kprintf("[TCP CA BBR] mode=%u bw=%u rtt=%u cwnd=%u\n",
            sk->ca.bbr.mode, sk->ca.bbr.lt_bw, sk->ca.bbr.min_rtt, sk->ca.cwnd);
}

void tcp_ca_bbr_on_loss(tcp_sock_t* sk) {
    /* BBR doesn't react to single losses like traditional algorithms */
    /* Only react if persistent congestion is detected */
    
    kprintf("[TCP CA BBR] Loss detected (ignoring)\n");
}

void tcp_ca_bbr_on_data_sent(tcp_sock_t* sk, uint32_t bytes) {
    /* Update round start time */
    uint32_t now = get_ticks();
    
    if (sk->ca.bbr.round_start == 0 || now - sk->ca.bbr.round_start > 100) {
        sk->ca.bbr.round_start = now;
    }
}

/* ==================== RTT Estimation ==================== */

void tcp_update_rtt(tcp_sock_t* sk) {
    if (!sk) return;
    
    uint32_t now = get_ticks();
    
    /* Calculate sample RTT (simplified - should use timestamps) */
    uint32_t sample_rtt = now - sk->retrans_timer;
    if (sample_rtt == 0 || sample_rtt > 10000) {
        /* Invalid sample */
        return;
    }
    
    sample_rtt *= 10;  /* Convert ticks to ms */
    
    if (sk->srtt == 0) {
        /* First measurement */
        sk->srtt = sample_rtt;
        sk->rttvar = sample_rtt / 2;
    } else {
        /* RFC 6298 */
        int32_t delta = sample_rtt - sk->srtt;
        sk->rttvar = (3 * sk->rttvar + abs(delta)) / 4;
        sk->srtt = (7 * sk->srtt + sample_rtt) / 8;
    }
    
    /* Calculate RTO: RTO = SRTT + max(G, K*RTTVAR) */
    sk->rto = sk->srtt + (4 * sk->rttvar);
    
    /* Clamp RTO */
    if (sk->rto < TCP_RTO_MIN) sk->rto = TCP_RTO_MIN;
    if (sk->rto > TCP_RTO_MAX) sk->rto = TCP_RTO_MAX;
    
    kprintf("[TCP RTT] sample=%u srtt=%u rttvar=%u rto=%u\n",
            sample_rtt, sk->srtt, sk->rttvar, sk->rto);
}

/* ==================== Retransmission Queue ==================== */

void tcp_add_retrans(tcp_sock_t* sk, struct sk_buff* skb, uint32_t seq) {
    if (!sk || !skb) return;
    
    tcp_retrans_entry_t* entry = (tcp_retrans_entry_t*)kmalloc(sizeof(tcp_retrans_entry_t));
    if (!entry) return;
    
    entry->skb = skb_clone(skb, 0);
    entry->seq = seq;
    entry->len = skb->len;
    entry->retrans_time = get_ticks();
    entry->next = NULL;
    
    /* Add to queue */
    if (!sk->retrans_queue) {
        sk->retrans_queue = entry;
    } else {
        tcp_retrans_entry_t* last = sk->retrans_queue;
        while (last->next) {
            last = last->next;
        }
        last->next = entry;
    }
}

void tcp_clean_retrans(tcp_sock_t* sk, uint32_t ack) {
    if (!sk) return;
    
    tcp_retrans_entry_t** prev = &sk->retrans_queue;
    tcp_retrans_entry_t* entry = sk->retrans_queue;
    
    while (entry) {
        if (entry->seq + entry->len <= ack) {
            /* This segment was acked */
            *prev = entry->next;
            free_skb(entry->skb);
            kfree(entry);
            entry = *prev;
        } else {
            prev = &entry->next;
            entry = entry->next;
        }
    }
}

void tcp_clear_retrans(tcp_sock_t* sk) {
    if (!sk) return;
    
    tcp_retrans_entry_t* entry = sk->retrans_queue;
    while (entry) {
        tcp_retrans_entry_t* next = entry->next;
        free_skb(entry->skb);
        kfree(entry);
        entry = next;
    }
    
    sk->retrans_queue = NULL;
}

void tcp_do_retransmit(tcp_sock_t* sk) {
    if (!sk || !sk->retrans_queue) return;
    
    kprintf("[TCP] Retransmitting segment seq=%u\n", sk->retrans_queue->seq);
    
    /* Retransmit first unacked segment */
    tcp_retrans_entry_t* entry = sk->retrans_queue;
    
    /* Clone packet and retransmit */
    struct sk_buff* skb = skb_clone(entry->skb, 0);
    if (skb) {
        /* Send via IP layer */
        ip_send(sk->remote_addr, skb);
        
        /* Update statistics */
        sk->retransmits++;
        
        /* Update RTO (exponential backoff) */
        sk->rto *= 2;
        if (sk->rto > TCP_RTO_MAX) {
            sk->rto = TCP_RTO_MAX;
        }
        
        /* Congestion control: timeout indicates loss */
        tcp_ca_on_loss(sk);
    }
    
    /* Update retransmission time */
    entry->retrans_time = get_ticks();
    
    /* Restart timer */
    sk->retrans_timer = get_ticks() + (sk->rto / 10);
}

/* ==================== Timer Management ==================== */

void tcp_timer_tick(void) {
    uint32_t now = get_ticks();
    
    /* Process all connected sockets */
    for (int i = 0; i < 1024; i++) {
        tcp_sock_t* sk = tcp_state.conn_hash[i];
        
        while (sk) {
            tcp_sock_t* next = sk->hash_next;
            
            /* Retransmission timer */
            if (sk->retrans_timer != 0 && now >= sk->retrans_timer) {
                tcp_do_retransmit(sk);
            }
            
            /* Delayed ACK timer */
            if (sk->delack_timer != 0 && now >= sk->delack_timer) {
                tcp_send_ack(sk);
                sk->delack_timer = 0;
            }
            
            /* Keepalive timer */
            if (sk->keepalive && sk->keepalive_timer != 0 && now >= sk->keepalive_timer) {
                /* Send keepalive probe */
                tcp_send_ack(sk);
                sk->keepalive_timer = now + (TCP_KEEPALIVE_TIME / 10);
            }
            
            /* TIME-WAIT timer */
            if (sk->state == TCP_TIME_WAIT && now >= sk->timewait_timer) {
                kprintf("[TCP] TIME-WAIT timeout, destroying socket\n");
                tcp_set_state(sk, TCP_CLOSED);
                tcp_socket_destroy(sk);
            }
            
            sk = next;
        }
    }
}

/* ==================== Helper Functions ==================== */

extern struct {
    tcp_sock_t* listen_hash[256];
    tcp_sock_t* conn_hash[1024];
    uint32_t isn_secret;
    tcp_stats_t stats;
    uint32_t lock;
} tcp_state;

/* Absolute value */
static int abs(int x) {
    return x < 0 ? -x : x;
}
