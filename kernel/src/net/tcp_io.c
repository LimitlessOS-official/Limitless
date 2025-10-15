/*
 * TCP Implementation - Part 2: Transmission and Reception
 * 
 * This file implements TCP packet transmission, reception, and data transfer.
 * 
 * Copyright (c) 2025 LimitlessOS Project
 */

#include "net/tcp_full.h"
#include "net/ip.h"
#include "net/skbuff.h"
#include "kernel.h"
#include <string.h>

/* ==================== Packet Transmission ==================== */

int tcp_transmit_skb(tcp_sock_t* sk, struct sk_buff* skb, uint32_t seq, uint32_t ack, uint16_t flags) {
    if (!sk || !skb) return -1;
    
    /* Reserve space for headers */
    skb_reserve(skb, sizeof(tcphdr_t) + sizeof(iphdr_t) + 14);  /* TCP + IP + Ethernet */
    
    /* Build TCP header */
    tcphdr_t* th = (tcphdr_t*)skb_push(skb, sizeof(tcphdr_t));
    memset(th, 0, sizeof(tcphdr_t));
    
    th->source = htons(sk->local_port);
    th->dest = htons(sk->remote_port);
    th->seq = htonl(seq);
    th->ack_seq = htonl(ack);
    th->doff = 5;  /* 20 bytes (no options for now) */
    th->window = htons(sk->rcv_wnd >> sk->rcv_wscale);
    
    /* Set flags */
    if (flags & TCP_FLAG_FIN) th->fin = 1;
    if (flags & TCP_FLAG_SYN) th->syn = 1;
    if (flags & TCP_FLAG_RST) th->rst = 1;
    if (flags & TCP_FLAG_PSH) th->psh = 1;
    if (flags & TCP_FLAG_ACK) th->ack = 1;
    if (flags & TCP_FLAG_URG) th->urg = 1;
    
    /* Calculate checksum */
    th->check = tcp_checksum(sk, skb);
    
    /* Set skb metadata */
    skb->protocol = IPPROTO_TCP;
    skb->sk = sk;
    
    /* Send via IP layer */
    int ret = ip_send(sk->remote_addr, skb);
    
    if (ret == 0) {
        sk->segments_out++;
        sk->bytes_out += skb->len;
        
        kprintf("[TCP] Sent segment: seq=%u ack=%u flags=%s%s%s%s len=%u\n",
                seq, ack,
                (flags & TCP_FLAG_SYN) ? "SYN " : "",
                (flags & TCP_FLAG_ACK) ? "ACK " : "",
                (flags & TCP_FLAG_FIN) ? "FIN " : "",
                (flags & TCP_FLAG_RST) ? "RST " : "",
                skb->len);
    }
    
    return ret;
}

int tcp_send_syn(tcp_sock_t* sk) {
    if (!sk) return -1;
    
    struct sk_buff* skb = alloc_skb(0, 0);
    if (!skb) return -1;
    
    /* Send SYN with MSS option */
    uint16_t flags = TCP_FLAG_SYN;
    
    int ret = tcp_transmit_skb(sk, skb, sk->iss, 0, flags);
    
    /* Start retransmission timer */
    sk->retrans_timer = get_ticks() + (sk->rto / 10);
    
    return ret;
}

int tcp_send_synack(tcp_sock_t* sk) {
    if (!sk) return -1;
    
    struct sk_buff* skb = alloc_skb(0, 0);
    if (!skb) return -1;
    
    uint16_t flags = TCP_FLAG_SYN | TCP_FLAG_ACK;
    
    int ret = tcp_transmit_skb(sk, skb, sk->iss, sk->rcv_nxt, flags);
    
    /* Start retransmission timer */
    sk->retrans_timer = get_ticks() + (sk->rto / 10);
    
    return ret;
}

int tcp_send_ack(tcp_sock_t* sk) {
    if (!sk) return -1;
    
    struct sk_buff* skb = alloc_skb(0, 0);
    if (!skb) return -1;
    
    uint16_t flags = TCP_FLAG_ACK;
    
    return tcp_transmit_skb(skb, sk, sk->snd_nxt, sk->rcv_nxt, flags);
}

int tcp_send_fin(tcp_sock_t* sk) {
    if (!sk) return -1;
    
    struct sk_buff* skb = alloc_skb(0, 0);
    if (!skb) return -1;
    
    uint16_t flags = TCP_FLAG_FIN | TCP_FLAG_ACK;
    
    /* FIN consumes one sequence number */
    int ret = tcp_transmit_skb(sk, skb, sk->snd_nxt, sk->rcv_nxt, flags);
    if (ret == 0) {
        sk->snd_nxt++;
    }
    
    /* Start retransmission timer */
    sk->retrans_timer = get_ticks() + (sk->rto / 10);
    
    return ret;
}

int tcp_send_reset(tcp_sock_t* sk, uint32_t seq, uint32_t ack) {
    struct sk_buff* skb = alloc_skb(0, 0);
    if (!skb) return -1;
    
    uint16_t flags = TCP_FLAG_RST | TCP_FLAG_ACK;
    
    return tcp_transmit_skb(sk, skb, seq, ack, flags);
}

int tcp_send_data(tcp_sock_t* sk, const void* data, uint32_t len, uint16_t flags) {
    if (!sk || !data || len == 0) return -1;
    
    if (sk->state != TCP_ESTABLISHED && sk->state != TCP_CLOSE_WAIT) {
        kprintf("[TCP] Cannot send data in state %s\n", tcp_state_str(sk->state));
        return -1;
    }
    
    /* Check window */
    uint32_t window = sk->snd_una + sk->snd_wnd - sk->snd_nxt;
    if (window == 0) {
        kprintf("[TCP] Send window closed\n");
        return -2;  /* Window full */
    }
    
    /* Limit to MSS and window */
    if (len > sk->mss) len = sk->mss;
    if (len > window) len = window;
    
    /* Allocate skb and copy data */
    struct sk_buff* skb = alloc_skb(len, 0);
    if (!skb) return -1;
    
    memcpy(skb_put(skb, len), data, len);
    
    /* Add PSH flag if requested */
    if (flags & TCP_FLAG_PSH) {
        flags |= TCP_FLAG_ACK;
    } else {
        flags = TCP_FLAG_ACK;
    }
    
    int ret = tcp_transmit_skb(sk, skb, sk->snd_nxt, sk->rcv_nxt, flags);
    
    if (ret == 0) {
        /* Update send sequence */
        sk->snd_nxt += len;
        
        /* Add to retransmission queue */
        tcp_add_retrans(sk, skb, sk->snd_nxt - len);
        
        /* Start retransmission timer if not running */
        if (sk->retrans_timer == 0) {
            sk->retrans_timer = get_ticks() + (sk->rto / 10);
        }
        
        /* Update congestion window */
        tcp_ca_on_data_sent(sk, len);
    } else {
        free_skb(skb);
    }
    
    return ret == 0 ? (int)len : ret;
}

/* ==================== Packet Reception ==================== */

void tcp_rcv(struct sk_buff* skb) {
    if (!skb || skb->len < sizeof(tcphdr_t)) {
        kprintf("[TCP] Invalid packet (too small)\n");
        free_skb(skb);
        return;
    }
    
    /* Get IP header for addresses */
    iphdr_t* iph = (iphdr_t*)skb->nh.raw;
    ipv4_addr_t saddr = ntohl(iph->saddr);
    ipv4_addr_t daddr = ntohl(iph->daddr);
    
    /* Get TCP header */
    tcphdr_t* th = (tcphdr_t*)skb->h.raw;
    uint16_t sport = ntohs(th->source);
    uint16_t dport = ntohs(th->dest);
    uint32_t seq = ntohl(th->seq);
    uint32_t ack = ntohl(th->ack_seq);
    uint16_t window = ntohs(th->window);
    
    /* Verify checksum */
    if (tcp_verify_checksum(skb)) {
        kprintf("[TCP] Checksum failed\n");
        free_skb(skb);
        return;
    }
    
    kprintf("[TCP] Received: %s:%u -> %s:%u seq=%u ack=%u flags=%s%s%s%s len=%u\n",
            ip_addr_to_str(saddr, NULL, 0), sport,
            ip_addr_to_str(daddr, NULL, 0), dport,
            seq, ack,
            th->syn ? "SYN " : "",
            th->ack ? "ACK " : "",
            th->fin ? "FIN " : "",
            th->rst ? "RST " : "",
            skb->len - (th->doff * 4));
    
    /* Look up socket */
    tcp_sock_t* sk = tcp_lookup(saddr, sport, daddr, dport);
    
    if (!sk) {
        /* Check for listening socket */
        sk = tcp_lookup_listen(daddr, dport);
        
        if (!sk) {
            kprintf("[TCP] No socket found, sending RST\n");
            /* Send RST */
            tcp_send_reset(NULL, ack, seq + 1);
            free_skb(skb);
            return;
        }
    }
    
    /* Update statistics */
    sk->segments_in++;
    sk->bytes_in += skb->len;
    
    /* Process based on state */
    tcp_process_segment(sk, skb, th, seq, ack, window);
}

void tcp_process_segment(tcp_sock_t* sk, struct sk_buff* skb, tcphdr_t* th,
                        uint32_t seq, uint32_t ack, uint16_t window) {
    if (!sk || !skb || !th) return;
    
    /* Handle RST */
    if (th->rst) {
        kprintf("[TCP] Received RST, closing connection\n");
        tcp_set_state(sk, TCP_CLOSED);
        tcp_socket_destroy(sk);
        free_skb(skb);
        return;
    }
    
    /* Process based on state */
    switch (sk->state) {
        case TCP_LISTEN:
            tcp_process_listen(sk, skb, th, seq, ack, window);
            break;
            
        case TCP_SYN_SENT:
            tcp_process_syn_sent(sk, skb, th, seq, ack, window);
            break;
            
        case TCP_SYN_RECV:
            tcp_process_syn_recv(sk, skb, th, seq, ack, window);
            break;
            
        case TCP_ESTABLISHED:
            tcp_process_established(sk, skb, th, seq, ack, window);
            break;
            
        case TCP_FIN_WAIT1:
        case TCP_FIN_WAIT2:
        case TCP_CLOSING:
            tcp_process_fin_wait(sk, skb, th, seq, ack, window);
            break;
            
        case TCP_CLOSE_WAIT:
            tcp_process_close_wait(sk, skb, th, seq, ack, window);
            break;
            
        case TCP_LAST_ACK:
            tcp_process_last_ack(sk, skb, th, seq, ack, window);
            break;
            
        case TCP_TIME_WAIT:
            tcp_process_time_wait(sk, skb, th, seq, ack, window);
            break;
            
        default:
            kprintf("[TCP] Unexpected state: %s\n", tcp_state_str(sk->state));
            free_skb(skb);
            break;
    }
}

/* ==================== State-specific Processing ==================== */

void tcp_process_listen(tcp_sock_t* sk, struct sk_buff* skb, tcphdr_t* th,
                       uint32_t seq, uint32_t ack, uint16_t window) {
    /* Only accept SYN */
    if (!th->syn) {
        kprintf("[TCP] Non-SYN packet in LISTEN state\n");
        free_skb(skb);
        return;
    }
    
    /* Check listen queue */
    if (sk->listen.qlen >= sk->listen.max_qlen) {
        kprintf("[TCP] Listen queue full, dropping SYN\n");
        free_skb(skb);
        return;
    }
    
    /* Get IP header for addresses */
    iphdr_t* iph = (iphdr_t*)skb->nh.raw;
    ipv4_addr_t saddr = ntohl(iph->saddr);
    uint16_t sport = ntohs(th->source);
    
    /* Create new socket for connection */
    tcp_sock_t* new_sk = tcp_socket_create();
    if (!new_sk) {
        kprintf("[TCP] Failed to create new socket\n");
        free_skb(skb);
        return;
    }
    
    /* Initialize new socket */
    new_sk->local_addr = sk->local_addr;
    new_sk->local_port = sk->local_port;
    new_sk->remote_addr = saddr;
    new_sk->remote_port = sport;
    
    /* Initialize sequence numbers */
    new_sk->rcv_nxt = seq + 1;
    new_sk->irs = seq;
    new_sk->iss = tcp_generate_isn(new_sk);
    new_sk->snd_una = new_sk->iss;
    new_sk->snd_nxt = new_sk->iss + 1;
    
    /* Update window */
    new_sk->snd_wnd = window;
    
    /* Move to SYN_RECV state */
    tcp_set_state(new_sk, TCP_SYN_RECV);
    
    /* Add to hash table */
    tcp_hash(new_sk);
    
    /* Send SYN-ACK */
    tcp_send_synack(new_sk);
    
    free_skb(skb);
}

void tcp_process_syn_sent(tcp_sock_t* sk, struct sk_buff* skb, tcphdr_t* th,
                         uint32_t seq, uint32_t ack, uint16_t window) {
    /* Expect SYN-ACK */
    if (!th->syn || !th->ack) {
        kprintf("[TCP] Unexpected flags in SYN_SENT state\n");
        free_skb(skb);
        return;
    }
    
    /* Verify ACK */
    if (ack != sk->iss + 1) {
        kprintf("[TCP] Invalid ACK in SYN_SENT: expected %u, got %u\n",
                sk->iss + 1, ack);
        free_skb(skb);
        return;
    }
    
    /* Initialize receive sequence */
    sk->rcv_nxt = seq + 1;
    sk->irs = seq;
    sk->snd_una = ack;
    
    /* Update window */
    sk->snd_wnd = window;
    
    /* Move to ESTABLISHED state */
    tcp_set_state(sk, TCP_ESTABLISHED);
    
    /* Send ACK */
    tcp_send_ack(sk);
    
    /* Cancel retransmission timer */
    sk->retrans_timer = 0;
    
    kprintf("[TCP] Connection established\n");
    
    free_skb(skb);
}

void tcp_process_syn_recv(tcp_sock_t* sk, struct sk_buff* skb, tcphdr_t* th,
                         uint32_t seq, uint32_t ack, uint16_t window) {
    /* Expect ACK */
    if (!th->ack) {
        kprintf("[TCP] No ACK in SYN_RECV state\n");
        free_skb(skb);
        return;
    }
    
    /* Verify ACK */
    if (ack != sk->iss + 1) {
        kprintf("[TCP] Invalid ACK in SYN_RECV\n");
        free_skb(skb);
        return;
    }
    
    /* Update state */
    sk->snd_una = ack;
    sk->snd_wnd = window;
    
    /* Move to ESTABLISHED state */
    tcp_set_state(sk, TCP_ESTABLISHED);
    
    /* Add to listen queue for accept() */
    /* Find parent listening socket */
    tcp_sock_t* listen_sk = tcp_lookup_listen(sk->local_addr, sk->local_port);
    if (listen_sk && listen_sk->listen.qlen < listen_sk->listen.max_qlen) {
        listen_sk->listen.queue[listen_sk->listen.qlen++] = sk;
        kprintf("[TCP] Added to listen queue (qlen=%u)\n", listen_sk->listen.qlen);
    }
    
    /* Cancel retransmission timer */
    sk->retrans_timer = 0;
    
    kprintf("[TCP] Connection established (passive)\n");
    
    free_skb(skb);
}

void tcp_process_established(tcp_sock_t* sk, struct sk_buff* skb, tcphdr_t* th,
                            uint32_t seq, uint32_t ack, uint16_t window) {
    uint32_t data_len = skb->len - (th->doff * 4);
    
    /* Process ACK */
    if (th->ack && ack > sk->snd_una) {
        uint32_t acked = ack - sk->snd_una;
        sk->snd_una = ack;
        
        /* Update RTT and RTO */
        tcp_update_rtt(sk);
        
        /* Remove acked segments from retransmission queue */
        tcp_clean_retrans(sk, ack);
        
        /* Update congestion window */
        tcp_ca_on_ack(sk, acked);
        
        /* If all data acked, cancel retransmission timer */
        if (sk->snd_una == sk->snd_nxt) {
            sk->retrans_timer = 0;
        }
    }
    
    /* Update window */
    sk->snd_wnd = window;
    
    /* Process data */
    if (data_len > 0) {
        if (seq == sk->rcv_nxt) {
            /* In-order data */
            uint8_t* data = (uint8_t*)th + (th->doff * 4);
            tcp_queue_data(sk, data, data_len);
            sk->rcv_nxt += data_len;
            
            /* Send ACK */
            tcp_send_ack(sk);
        } else if (seq > sk->rcv_nxt) {
            /* Out-of-order data */
            kprintf("[TCP] Out-of-order segment: seq=%u expected=%u\n",
                    seq, sk->rcv_nxt);
            /* Add to out-of-order queue */
            skb_queue_tail(&sk->ofo_queue, skb);
            skb = NULL;  /* Don't free */
            
            /* Send duplicate ACK */
            tcp_send_ack(sk);
        } else {
            /* Old data, already received */
            kprintf("[TCP] Duplicate segment: seq=%u\n", seq);
            /* Send ACK anyway */
            tcp_send_ack(sk);
        }
    }
    
    /* Check for FIN */
    if (th->fin) {
        kprintf("[TCP] Received FIN\n");
        sk->rcv_nxt++;  /* FIN consumes sequence number */
        
        /* Send ACK */
        tcp_send_ack(sk);
        
        /* Move to CLOSE_WAIT */
        tcp_set_state(sk, TCP_CLOSE_WAIT);
    }
    
    if (skb) free_skb(skb);
}

void tcp_process_fin_wait(tcp_sock_t* sk, struct sk_buff* skb, tcphdr_t* th,
                         uint32_t seq, uint32_t ack, uint16_t window) {
    /* Process ACK */
    if (th->ack && ack > sk->snd_una) {
        sk->snd_una = ack;
        
        if (sk->state == TCP_FIN_WAIT1 && ack == sk->snd_nxt) {
            /* Our FIN was acked */
            tcp_set_state(sk, TCP_FIN_WAIT2);
            sk->retrans_timer = 0;
        } else if (sk->state == TCP_CLOSING && ack == sk->snd_nxt) {
            /* Both FINs acked */
            tcp_set_state(sk, TCP_TIME_WAIT);
            sk->retrans_timer = 0;
        }
    }
    
    /* Check for FIN */
    if (th->fin) {
        sk->rcv_nxt++;
        tcp_send_ack(sk);
        
        if (sk->state == TCP_FIN_WAIT1) {
            /* Simultaneous close */
            tcp_set_state(sk, TCP_CLOSING);
        } else if (sk->state == TCP_FIN_WAIT2) {
            /* Normal close */
            tcp_set_state(sk, TCP_TIME_WAIT);
        }
    }
    
    free_skb(skb);
}

void tcp_process_close_wait(tcp_sock_t* sk, struct sk_buff* skb, tcphdr_t* th,
                           uint32_t seq, uint32_t ack, uint16_t window) {
    /* Just process ACKs */
    if (th->ack && ack > sk->snd_una) {
        sk->snd_una = ack;
    }
    
    free_skb(skb);
}

void tcp_process_last_ack(tcp_sock_t* sk, struct sk_buff* skb, tcphdr_t* th,
                         uint32_t seq, uint32_t ack, uint16_t window) {
    /* Wait for ACK of our FIN */
    if (th->ack && ack == sk->snd_nxt) {
        kprintf("[TCP] Final ACK received, closing\n");
        tcp_set_state(sk, TCP_CLOSED);
        sk->retrans_timer = 0;
        tcp_socket_destroy(sk);
    }
    
    free_skb(skb);
}

void tcp_process_time_wait(tcp_sock_t* sk, struct sk_buff* skb, tcphdr_t* th,
                          uint32_t seq, uint32_t ack, uint32_t window) {
    /* Restart TIME-WAIT timer if we get anything */
    sk->timewait_timer = get_ticks() + (TCP_TIMEWAIT_LEN / 10);
    
    free_skb(skb);
}

/* ==================== Data Queue Management ==================== */

void tcp_queue_data(tcp_sock_t* sk, const void* data, uint32_t len) {
    if (!sk || !data || len == 0) return;
    
    /* Allocate buffer */
    tcp_recv_buf_t* buf = (tcp_recv_buf_t*)kmalloc(sizeof(tcp_recv_buf_t));
    if (!buf) {
        kprintf("[TCP] Failed to allocate receive buffer\n");
        return;
    }
    
    buf->data = (uint8_t*)kmalloc(len);
    if (!buf->data) {
        kfree(buf);
        return;
    }
    
    memcpy(buf->data, data, len);
    buf->len = len;
    buf->next = NULL;
    
    /* Add to receive buffer */
    if (!sk->recv_buf) {
        sk->recv_buf = buf;
    } else {
        tcp_recv_buf_t* last = sk->recv_buf;
        while (last->next) {
            last = last->next;
        }
        last->next = buf;
    }
    
    kprintf("[TCP] Queued %u bytes of data\n", len);
}

int tcp_read_data(tcp_sock_t* sk, void* buffer, uint32_t len) {
    if (!sk || !buffer || len == 0) return -1;
    
    if (!sk->recv_buf) {
        return 0;  /* No data available */
    }
    
    uint32_t copied = 0;
    uint8_t* dest = (uint8_t*)buffer;
    
    while (sk->recv_buf && copied < len) {
        tcp_recv_buf_t* buf = sk->recv_buf;
        uint32_t to_copy = (buf->len < (len - copied)) ? buf->len : (len - copied);
        
        memcpy(dest + copied, buf->data, to_copy);
        copied += to_copy;
        
        if (to_copy == buf->len) {
            /* Buffer fully consumed */
            sk->recv_buf = buf->next;
            kfree(buf->data);
            kfree(buf);
        } else {
            /* Partial read */
            memmove(buf->data, buf->data + to_copy, buf->len - to_copy);
            buf->len -= to_copy;
        }
    }
    
    return copied;
}

/* ==================== Checksum ==================== */

uint16_t tcp_checksum(tcp_sock_t* sk, struct sk_buff* skb) {
    /* Simplified checksum - should include pseudo-header */
    tcphdr_t* th = (tcphdr_t*)skb->h.raw;
    uint32_t sum = 0;
    uint16_t* data = (uint16_t*)th;
    uint32_t len = skb->len;
    
    /* Checksum TCP header and data */
    while (len > 1) {
        sum += *data++;
        len -= 2;
    }
    
    /* Add odd byte if present */
    if (len == 1) {
        sum += *((uint8_t*)data);
    }
    
    /* Fold to 16 bits */
    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }
    
    return ~sum;
}

int tcp_verify_checksum(struct sk_buff* skb) {
    /* TODO: Implement proper checksum verification */
    return 0;  /* Accept all for now */
}
