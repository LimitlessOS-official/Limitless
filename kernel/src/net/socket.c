/*
 * BSD Socket API Implementation
 * POSIX Socket Interface
 * 
 * Copyright (c) 2025 LimitlessOS Project
 */

#include "net/socket.h"
#include "net/tcp.h"
#include "net/udp.h"
#include "net/ip.h"
#include "kernel/printk.h"
#include "kernel/string.h"
#include "kernel/stdlib.h"
#include "kernel/errno.h"
#include "fs/vfs.h"

#define MAX_SOCKETS 1024

/* Socket state */
typedef enum socket_state {
    SS_FREE = 0,                 /* Not allocated */
    SS_UNCONNECTED,              /* Unconnected */
    SS_CONNECTING,               /* In process of connecting */
    SS_CONNECTED,                /* Connected */
    SS_DISCONNECTING,            /* In process of disconnecting */
} socket_state_t;

/* Internal socket structure */
typedef struct socket {
    int                 fd;             /* File descriptor */
    int                 family;         /* Address family (AF_INET, etc.) */
    int                 type;           /* Socket type (SOCK_STREAM, etc.) */
    int                 protocol;       /* Protocol (IPPROTO_TCP, etc.) */
    socket_state_t      state;          /* Connection state */
    
    /* Addresses */
    struct sockaddr_in  local_addr;     /* Local address */
    struct sockaddr_in  peer_addr;      /* Peer address */
    
    /* Socket options */
    int                 reuse_addr;     /* SO_REUSEADDR */
    int                 reuse_port;     /* SO_REUSEPORT */
    int                 broadcast;      /* SO_BROADCAST */
    int                 keepalive;      /* SO_KEEPALIVE */
    int                 nonblocking;    /* Non-blocking mode */
    int                 linger_on;      /* SO_LINGER */
    int                 linger_time;    /* Linger time in seconds */
    uint32_t            send_bufsize;   /* SO_SNDBUF */
    uint32_t            recv_bufsize;   /* SO_RCVBUF */
    int                 send_timeout;   /* SO_SNDTIMEO (ms) */
    int                 recv_timeout;   /* SO_RCVTIMEO (ms) */
    
    /* TCP-specific */
    tcp_socket_t*       tcp_sock;       /* TCP socket data */
    
    /* UDP-specific */
    udp_socket_t*       udp_sock;       /* UDP socket data */
    
    /* Listen backlog */
    int                 backlog;        /* Maximum pending connections */
    struct socket**     accept_queue;   /* Queue of accepted connections */
    int                 accept_queue_len; /* Number of pending connections */
    
    /* Statistics */
    uint64_t            bytes_sent;
    uint64_t            bytes_received;
    
} socket_t;

/* Global socket table */
static socket_t* socket_table[MAX_SOCKETS] = {0};
static int next_fd = 3;  /* Start after stdin/stdout/stderr */

/* Helper: Find free socket slot */
static int socket_alloc_fd(void) {
    int i;
    for (i = 0; i < MAX_SOCKETS; i++) {
        if (!socket_table[i]) {
            int fd = next_fd++;
            socket_table[i] = (socket_t*)malloc(sizeof(socket_t));
            if (!socket_table[i]) {
                return -ENOMEM;
            }
            memset(socket_table[i], 0, sizeof(socket_t));
            socket_table[i]->fd = fd;
            socket_table[i]->state = SS_UNCONNECTED;
            socket_table[i]->send_bufsize = 16384;
            socket_table[i]->recv_bufsize = 16384;
            return fd;
        }
    }
    return -EMFILE;  /* Too many open files */
}

/* Helper: Get socket by FD */
static socket_t* socket_get_by_fd(int sockfd) {
    int i;
    for (i = 0; i < MAX_SOCKETS; i++) {
        if (socket_table[i] && socket_table[i]->fd == sockfd) {
            return socket_table[i];
        }
    }
    return NULL;
}

/* Helper: Free socket slot */
static void socket_free_fd(int sockfd) {
    int i;
    for (i = 0; i < MAX_SOCKETS; i++) {
        if (socket_table[i] && socket_table[i]->fd == sockfd) {
            if (socket_table[i]->tcp_sock) {
                /* Close TCP socket */
                tcp_close(socket_table[i]->tcp_sock);
            }
            if (socket_table[i]->udp_sock) {
                /* Close UDP socket */
                udp_close(socket_table[i]->udp_sock);
            }
            if (socket_table[i]->accept_queue) {
                free(socket_table[i]->accept_queue);
            }
            free(socket_table[i]);
            socket_table[i] = NULL;
            return;
        }
    }
}

/*
 * Create a socket
 * 
 * @domain: Address family (AF_INET, etc.)
 * @type: Socket type (SOCK_STREAM, SOCK_DGRAM, etc.)
 * @protocol: Protocol (IPPROTO_TCP, IPPROTO_UDP, etc.)
 * @return: Socket file descriptor on success, negative on error
 */
int socket(int domain, int type, int protocol) {
    socket_t* sock;
    int fd;
    
    /* Validate parameters */
    if (domain != AF_INET) {
        return -EAFNOSUPPORT;  /* Only IPv4 supported for now */
    }
    
    if (type != SOCK_STREAM && type != SOCK_DGRAM && type != SOCK_RAW) {
        return -EINVAL;
    }
    
    /* Allocate socket */
    fd = socket_alloc_fd();
    if (fd < 0) {
        return fd;
    }
    
    sock = socket_get_by_fd(fd);
    if (!sock) {
        return -ENOMEM;
    }
    
    sock->family = domain;
    sock->type = type;
    sock->protocol = protocol;
    
    /* Create protocol-specific socket */
    if (type == SOCK_STREAM) {
        /* TCP socket */
        sock->tcp_sock = tcp_create();
        if (!sock->tcp_sock) {
            socket_free_fd(fd);
            return -ENOMEM;
        }
    } else if (type == SOCK_DGRAM) {
        /* UDP socket */
        sock->udp_sock = udp_create();
        if (!sock->udp_sock) {
            socket_free_fd(fd);
            return -ENOMEM;
        }
    }
    
    return fd;
}

/*
 * Bind socket to address
 * 
 * @sockfd: Socket file descriptor
 * @addr: Address to bind to
 * @addrlen: Length of address structure
 * @return: 0 on success, negative on error
 */
int bind(int sockfd, const struct sockaddr* addr, uint32_t addrlen) {
    socket_t* sock;
    const struct sockaddr_in* addr_in;
    int ret;
    
    sock = socket_get_by_fd(sockfd);
    if (!sock) {
        return -EBADF;
    }
    
    if (!addr || addrlen < sizeof(struct sockaddr_in)) {
        return -EINVAL;
    }
    
    addr_in = (const struct sockaddr_in*)addr;
    
    if (addr_in->sin_family != AF_INET) {
        return -EAFNOSUPPORT;
    }
    
    /* Save local address */
    memcpy(&sock->local_addr, addr_in, sizeof(struct sockaddr_in));
    
    /* Bind protocol-specific socket */
    if (sock->type == SOCK_STREAM) {
        ret = tcp_bind(sock->tcp_sock, ntohl(addr_in->sin_addr), ntohs(addr_in->sin_port));
    } else if (sock->type == SOCK_DGRAM) {
        ret = udp_bind(sock->udp_sock, ntohl(addr_in->sin_addr), ntohs(addr_in->sin_port));
    } else {
        return -EOPNOTSUPP;
    }
    
    return ret;
}

/*
 * Listen for connections
 * 
 * @sockfd: Socket file descriptor
 * @backlog: Maximum pending connections
 * @return: 0 on success, negative on error
 */
int listen(int sockfd, int backlog) {
    socket_t* sock;
    int ret;
    
    sock = socket_get_by_fd(sockfd);
    if (!sock) {
        return -EBADF;
    }
    
    if (sock->type != SOCK_STREAM) {
        return -EOPNOTSUPP;  /* Only TCP sockets can listen */
    }
    
    /* Allocate accept queue */
    if (backlog > SOMAXCONN) {
        backlog = SOMAXCONN;
    }
    
    sock->backlog = backlog;
    sock->accept_queue = (socket_t**)malloc(backlog * sizeof(socket_t*));
    if (!sock->accept_queue) {
        return -ENOMEM;
    }
    memset(sock->accept_queue, 0, backlog * sizeof(socket_t*));
    sock->accept_queue_len = 0;
    
    /* Start listening */
    ret = tcp_listen(sock->tcp_sock, backlog);
    if (ret < 0) {
        free(sock->accept_queue);
        sock->accept_queue = NULL;
        return ret;
    }
    
    return 0;
}

/*
 * Accept a connection
 * 
 * @sockfd: Socket file descriptor
 * @addr: Peer address (output)
 * @addrlen: Length of address structure (input/output)
 * @return: New socket FD on success, negative on error
 */
int accept(int sockfd, struct sockaddr* addr, uint32_t* addrlen) {
    socket_t* sock;
    socket_t* new_sock;
    int new_fd;
    tcp_socket_t* new_tcp_sock;
    
    sock = socket_get_by_fd(sockfd);
    if (!sock) {
        return -EBADF;
    }
    
    if (sock->type != SOCK_STREAM) {
        return -EOPNOTSUPP;
    }
    
    /* Accept TCP connection */
    new_tcp_sock = tcp_accept(sock->tcp_sock);
    if (!new_tcp_sock) {
        return sock->nonblocking ? -EAGAIN : -EINTR;
    }
    
    /* Create new socket for accepted connection */
    new_fd = socket_alloc_fd();
    if (new_fd < 0) {
        tcp_close(new_tcp_sock);
        return new_fd;
    }
    
    new_sock = socket_get_by_fd(new_fd);
    if (!new_sock) {
        tcp_close(new_tcp_sock);
        return -ENOMEM;
    }
    
    /* Initialize new socket */
    new_sock->family = sock->family;
    new_sock->type = sock->type;
    new_sock->protocol = sock->protocol;
    new_sock->state = SS_CONNECTED;
    new_sock->tcp_sock = new_tcp_sock;
    
    /* Get peer address from TCP socket */
    new_sock->peer_addr.sin_family = AF_INET;
    new_sock->peer_addr.sin_addr = htonl(new_tcp_sock->remote_ip);
    new_sock->peer_addr.sin_port = htons(new_tcp_sock->remote_port);
    
    /* Return peer address to caller */
    if (addr && addrlen) {
        uint32_t len = *addrlen < sizeof(struct sockaddr_in) ? 
                       *addrlen : sizeof(struct sockaddr_in);
        memcpy(addr, &new_sock->peer_addr, len);
        *addrlen = sizeof(struct sockaddr_in);
    }
    
    return new_fd;
}

/*
 * Connect to a remote address
 * 
 * @sockfd: Socket file descriptor
 * @addr: Remote address
 * @addrlen: Length of address structure
 * @return: 0 on success, negative on error
 */
int connect(int sockfd, const struct sockaddr* addr, uint32_t addrlen) {
    socket_t* sock;
    const struct sockaddr_in* addr_in;
    int ret;
    
    sock = socket_get_by_fd(sockfd);
    if (!sock) {
        return -EBADF;
    }
    
    if (!addr || addrlen < sizeof(struct sockaddr_in)) {
        return -EINVAL;
    }
    
    addr_in = (const struct sockaddr_in*)addr;
    
    if (addr_in->sin_family != AF_INET) {
        return -EAFNOSUPPORT;
    }
    
    /* Save peer address */
    memcpy(&sock->peer_addr, addr_in, sizeof(struct sockaddr_in));
    
    sock->state = SS_CONNECTING;
    
    /* Connect protocol-specific socket */
    if (sock->type == SOCK_STREAM) {
        ret = tcp_connect(sock->tcp_sock, ntohl(addr_in->sin_addr), ntohs(addr_in->sin_port));
        if (ret == 0) {
            sock->state = SS_CONNECTED;
        } else {
            sock->state = SS_UNCONNECTED;
        }
    } else if (sock->type == SOCK_DGRAM) {
        /* UDP "connect" just saves remote address */
        sock->state = SS_CONNECTED;
        ret = 0;
    } else {
        sock->state = SS_UNCONNECTED;
        return -EOPNOTSUPP;
    }
    
    return ret;
}

/*
 * Send data on a connected socket
 * 
 * @sockfd: Socket file descriptor
 * @buf: Data buffer
 * @len: Length of data
 * @flags: Send flags
 * @return: Number of bytes sent on success, negative on error
 */
ssize_t send(int sockfd, const void* buf, size_t len, int flags) {
    socket_t* sock;
    ssize_t ret;
    
    sock = socket_get_by_fd(sockfd);
    if (!sock) {
        return -EBADF;
    }
    
    if (!buf) {
        return -EINVAL;
    }
    
    if (sock->state != SS_CONNECTED) {
        return -ENOTCONN;
    }
    
    /* Send via protocol */
    if (sock->type == SOCK_STREAM) {
        ret = tcp_send(sock->tcp_sock, buf, len);
    } else if (sock->type == SOCK_DGRAM) {
        ret = udp_sendto(sock->udp_sock, buf, len,
                        ntohl(sock->peer_addr.sin_addr),
                        ntohs(sock->peer_addr.sin_port));
    } else {
        return -EOPNOTSUPP;
    }
    
    if (ret > 0) {
        sock->bytes_sent += ret;
    }
    
    return ret;
}

/*
 * Receive data from a connected socket
 * 
 * @sockfd: Socket file descriptor
 * @buf: Receive buffer
 * @len: Length of buffer
 * @flags: Receive flags
 * @return: Number of bytes received on success, negative on error
 */
ssize_t recv(int sockfd, void* buf, size_t len, int flags) {
    socket_t* sock;
    ssize_t ret;
    
    sock = socket_get_by_fd(sockfd);
    if (!sock) {
        return -EBADF;
    }
    
    if (!buf) {
        return -EINVAL;
    }
    
    if (sock->state != SS_CONNECTED) {
        return -ENOTCONN;
    }
    
    /* Receive via protocol */
    if (sock->type == SOCK_STREAM) {
        ret = tcp_recv(sock->tcp_sock, buf, len);
    } else if (sock->type == SOCK_DGRAM) {
        uint32_t src_ip, src_port;
        ret = udp_recvfrom(sock->udp_sock, buf, len, &src_ip, &src_port);
    } else {
        return -EOPNOTSUPP;
    }
    
    if (ret > 0) {
        sock->bytes_received += ret;
    }
    
    return ret;
}

/*
 * Send data to a specific address
 * 
 * @sockfd: Socket file descriptor
 * @buf: Data buffer
 * @len: Length of data
 * @flags: Send flags
 * @dest_addr: Destination address
 * @addrlen: Length of address structure
 * @return: Number of bytes sent on success, negative on error
 */
ssize_t sendto(int sockfd, const void* buf, size_t len, int flags,
               const struct sockaddr* dest_addr, uint32_t addrlen) {
    socket_t* sock;
    const struct sockaddr_in* addr_in;
    ssize_t ret;
    
    sock = socket_get_by_fd(sockfd);
    if (!sock) {
        return -EBADF;
    }
    
    if (!buf) {
        return -EINVAL;
    }
    
    /* If connected, use send() */
    if (sock->state == SS_CONNECTED && !dest_addr) {
        return send(sockfd, buf, len, flags);
    }
    
    if (!dest_addr || addrlen < sizeof(struct sockaddr_in)) {
        return -EINVAL;
    }
    
    addr_in = (const struct sockaddr_in*)dest_addr;
    
    /* Send via protocol */
    if (sock->type == SOCK_DGRAM) {
        ret = udp_sendto(sock->udp_sock, buf, len,
                        ntohl(addr_in->sin_addr),
                        ntohs(addr_in->sin_port));
    } else if (sock->type == SOCK_STREAM) {
        /* TCP doesn't support sendto on unconnected sockets */
        return -EISCONN;
    } else {
        return -EOPNOTSUPP;
    }
    
    if (ret > 0) {
        sock->bytes_sent += ret;
    }
    
    return ret;
}

/*
 * Receive data from any address
 * 
 * @sockfd: Socket file descriptor
 * @buf: Receive buffer
 * @len: Length of buffer
 * @flags: Receive flags
 * @src_addr: Source address (output)
 * @addrlen: Length of address structure (input/output)
 * @return: Number of bytes received on success, negative on error
 */
ssize_t recvfrom(int sockfd, void* buf, size_t len, int flags,
                 struct sockaddr* src_addr, uint32_t* addrlen) {
    socket_t* sock;
    struct sockaddr_in* addr_in;
    uint32_t src_ip, src_port;
    ssize_t ret;
    
    sock = socket_get_by_fd(sockfd);
    if (!sock) {
        return -EBADF;
    }
    
    if (!buf) {
        return -EINVAL;
    }
    
    /* Receive via protocol */
    if (sock->type == SOCK_DGRAM) {
        ret = udp_recvfrom(sock->udp_sock, buf, len, &src_ip, &src_port);
        
        /* Return source address */
        if (ret > 0 && src_addr && addrlen) {
            addr_in = (struct sockaddr_in*)src_addr;
            addr_in->sin_family = AF_INET;
            addr_in->sin_addr = htonl(src_ip);
            addr_in->sin_port = htons(src_port);
            memset(addr_in->sin_zero, 0, sizeof(addr_in->sin_zero));
            *addrlen = sizeof(struct sockaddr_in);
        }
    } else if (sock->type == SOCK_STREAM) {
        ret = tcp_recv(sock->tcp_sock, buf, len);
        
        /* Return peer address for TCP */
        if (ret > 0 && src_addr && addrlen) {
            memcpy(src_addr, &sock->peer_addr, sizeof(struct sockaddr_in));
            *addrlen = sizeof(struct sockaddr_in);
        }
    } else {
        return -EOPNOTSUPP;
    }
    
    if (ret > 0) {
        sock->bytes_received += ret;
    }
    
    return ret;
}

/*
 * Set socket options
 * 
 * @sockfd: Socket file descriptor
 * @level: Protocol level (SOL_SOCKET, SOL_TCP, etc.)
 * @optname: Option name
 * @optval: Option value
 * @optlen: Length of option value
 * @return: 0 on success, negative on error
 */
int setsockopt(int sockfd, int level, int optname,
               const void* optval, uint32_t optlen) {
    socket_t* sock;
    
    sock = socket_get_by_fd(sockfd);
    if (!sock) {
        return -EBADF;
    }
    
    if (!optval) {
        return -EINVAL;
    }
    
    /* Socket-level options */
    if (level == SOL_SOCKET) {
        switch (optname) {
            case SO_REUSEADDR:
                if (optlen < sizeof(int)) return -EINVAL;
                sock->reuse_addr = *(int*)optval;
                return 0;
                
            case SO_REUSEPORT:
                if (optlen < sizeof(int)) return -EINVAL;
                sock->reuse_port = *(int*)optval;
                return 0;
                
            case SO_BROADCAST:
                if (optlen < sizeof(int)) return -EINVAL;
                sock->broadcast = *(int*)optval;
                return 0;
                
            case SO_KEEPALIVE:
                if (optlen < sizeof(int)) return -EINVAL;
                sock->keepalive = *(int*)optval;
                return 0;
                
            case SO_SNDBUF:
                if (optlen < sizeof(int)) return -EINVAL;
                sock->send_bufsize = *(uint32_t*)optval;
                return 0;
                
            case SO_RCVBUF:
                if (optlen < sizeof(int)) return -EINVAL;
                sock->recv_bufsize = *(uint32_t*)optval;
                return 0;
                
            case SO_SNDTIMEO:
            case SO_RCVTIMEO: {
                struct timeval* tv = (struct timeval*)optval;
                if (optlen < sizeof(struct timeval)) return -EINVAL;
                int timeout_ms = tv->tv_sec * 1000 + tv->tv_usec / 1000;
                if (optname == SO_SNDTIMEO) {
                    sock->send_timeout = timeout_ms;
                } else {
                    sock->recv_timeout = timeout_ms;
                }
                return 0;
            }
                
            case SO_LINGER: {
                struct linger* lg = (struct linger*)optval;
                if (optlen < sizeof(struct linger)) return -EINVAL;
                sock->linger_on = lg->l_onoff;
                sock->linger_time = lg->l_linger;
                return 0;
            }
                
            default:
                return -ENOPROTOOPT;
        }
    }
    
    /* TCP-level options */
    if (level == SOL_TCP && sock->type == SOCK_STREAM) {
        return tcp_setsockopt(sock->tcp_sock, optname, optval, optlen);
    }
    
    return -ENOPROTOOPT;
}

/*
 * Get socket options
 * 
 * @sockfd: Socket file descriptor
 * @level: Protocol level
 * @optname: Option name
 * @optval: Option value (output)
 * @optlen: Length of option value (input/output)
 * @return: 0 on success, negative on error
 */
int getsockopt(int sockfd, int level, int optname,
               void* optval, uint32_t* optlen) {
    socket_t* sock;
    
    sock = socket_get_by_fd(sockfd);
    if (!sock) {
        return -EBADF;
    }
    
    if (!optval || !optlen) {
        return -EINVAL;
    }
    
    /* Socket-level options */
    if (level == SOL_SOCKET) {
        switch (optname) {
            case SO_TYPE:
                if (*optlen < sizeof(int)) return -EINVAL;
                *(int*)optval = sock->type;
                *optlen = sizeof(int);
                return 0;
                
            case SO_ERROR:
                if (*optlen < sizeof(int)) return -EINVAL;
                *(int*)optval = 0;  /* No error */
                *optlen = sizeof(int);
                return 0;
                
            case SO_REUSEADDR:
                if (*optlen < sizeof(int)) return -EINVAL;
                *(int*)optval = sock->reuse_addr;
                *optlen = sizeof(int);
                return 0;
                
            default:
                return -ENOPROTOOPT;
        }
    }
    
    return -ENOPROTOOPT;
}

/*
 * Shutdown socket
 * 
 * @sockfd: Socket file descriptor
 * @how: Shutdown mode (SHUT_RD, SHUT_WR, SHUT_RDWR)
 * @return: 0 on success, negative on error
 */
int shutdown(int sockfd, int how) {
    socket_t* sock;
    
    sock = socket_get_by_fd(sockfd);
    if (!sock) {
        return -EBADF;
    }
    
    if (sock->type == SOCK_STREAM && sock->tcp_sock) {
        return tcp_shutdown(sock->tcp_sock, how);
    }
    
    return 0;
}

/*
 * Close socket
 * 
 * @sockfd: Socket file descriptor
 * @return: 0 on success, negative on error
 */
int close(int sockfd) {
    socket_t* sock;
    
    sock = socket_get_by_fd(sockfd);
    if (!sock) {
        return -EBADF;
    }
    
    socket_free_fd(sockfd);
    return 0;
}
