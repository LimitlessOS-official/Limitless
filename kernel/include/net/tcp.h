#pragma once
#include "kernel.h"
#include "net/net.h"

void tcp_init(void);
void tcp_timer_tick(void);  /* Call periodically for timeout processing */

/* TCP socket operations */
int  tcp_bind(sock_t s, const void* addr, u32 addrlen);
int  tcp_listen(sock_t s, int backlog);
int  tcp_connect(sock_t s, const void* addr, u32 addrlen);
int  tcp_send(sock_t s, const void* buf, u32 len);
int  tcp_recv(sock_t s, void* buf, u32 len);
int  tcp_close(sock_t s);