/*
 * BSD Socket API Header
 * POSIX Socket Interface
 * 
 * Copyright (c) 2025 LimitlessOS Project
 */

#pragma once
#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Socket Address Families */
#define AF_UNSPEC       0        /* Unspecified */
#define AF_UNIX         1        /* Unix domain sockets */
#define AF_LOCAL        AF_UNIX  /* POSIX name for AF_UNIX */
#define AF_INET         2        /* Internet IP Protocol */
#define AF_AX25         3        /* Amateur Radio AX.25 */
#define AF_IPX          4        /* Novell IPX */
#define AF_APPLETALK    5        /* AppleTalk DDP */
#define AF_NETROM       6        /* Amateur Radio NET/ROM */
#define AF_BRIDGE       7        /* Multiprotocol bridge */
#define AF_ATMPVC       8        /* ATM PVCs */
#define AF_X25          9        /* Reserved for X.25 project */
#define AF_INET6        10       /* IP version 6 */
#define AF_PACKET       17       /* Packet family */
#define AF_MAX          32       /* Maximum address family */

/* Protocol Families (same as AF_*) */
#define PF_UNSPEC       AF_UNSPEC
#define PF_UNIX         AF_UNIX
#define PF_LOCAL        AF_LOCAL
#define PF_INET         AF_INET
#define PF_INET6        AF_INET6
#define PF_PACKET       AF_PACKET

/* Socket Types */
#define SOCK_STREAM     1        /* Stream socket (TCP) */
#define SOCK_DGRAM      2        /* Datagram socket (UDP) */
#define SOCK_RAW        3        /* Raw protocol interface */
#define SOCK_RDM        4        /* Reliably-delivered message */
#define SOCK_SEQPACKET  5        /* Sequential packet socket */
#define SOCK_PACKET     10       /* Linux specific */

/* Socket Protocol */
#define IPPROTO_IP      0        /* Dummy protocol for TCP */
#define IPPROTO_ICMP    1        /* Internet Control Message Protocol */
#define IPPROTO_IGMP    2        /* Internet Group Management Protocol */
#define IPPROTO_IPIP    4        /* IPIP tunnels (encapsulation) */
#define IPPROTO_TCP     6        /* Transmission Control Protocol */
#define IPPROTO_EGP     8        /* Exterior Gateway Protocol */
#define IPPROTO_PUP     12       /* PUP protocol */
#define IPPROTO_UDP     17       /* User Datagram Protocol */
#define IPPROTO_IDP     22       /* XNS IDP protocol */
#define IPPROTO_TP      29       /* SO Transport Protocol Class 4 */
#define IPPROTO_DCCP    33       /* Datagram Congestion Control Protocol */
#define IPPROTO_IPV6    41       /* IPv6-in-IPv4 tunnelling */
#define IPPROTO_RSVP    46       /* RSVP Protocol */
#define IPPROTO_GRE     47       /* Cisco GRE tunnels */
#define IPPROTO_ESP     50       /* Encapsulation Security Payload protocol */
#define IPPROTO_AH      51       /* Authentication Header protocol */
#define IPPROTO_MTP     92       /* Multicast Transport Protocol */
#define IPPROTO_ENCAP   98       /* Encapsulation Header */
#define IPPROTO_PIM     103      /* Protocol Independent Multicast */
#define IPPROTO_SCTP    132      /* Stream Control Transport Protocol */
#define IPPROTO_RAW     255      /* Raw IP packets */

/* Socket Options - Level */
#define SOL_SOCKET      1        /* Socket-level options */
#define SOL_IP          0        /* IP-level options */
#define SOL_TCP         6        /* TCP-level options */
#define SOL_UDP         17       /* UDP-level options */

/* Socket Options - SOL_SOCKET */
#define SO_DEBUG        1        /* Debugging info recording */
#define SO_REUSEADDR    2        /* Reuse local addresses */
#define SO_TYPE         3        /* Get socket type */
#define SO_ERROR        4        /* Get error status and clear */
#define SO_DONTROUTE    5        /* Don't use routing */
#define SO_BROADCAST    6        /* Permit sending of broadcast msgs */
#define SO_SNDBUF       7        /* Send buffer size */
#define SO_RCVBUF       8        /* Receive buffer size */
#define SO_KEEPALIVE    9        /* Keep connections alive */
#define SO_OOBINLINE    10       /* Leave received OOB data in line */
#define SO_NO_CHECK     11       /* Disable checksums */
#define SO_PRIORITY     12       /* Socket priority */
#define SO_LINGER       13       /* Linger on close if data present */
#define SO_BSDCOMPAT    14       /* BSD compatibility */
#define SO_REUSEPORT    15       /* Allow multiple binds to same port */
#define SO_RCVLOWAT     18       /* Receive low-water mark */
#define SO_SNDLOWAT     19       /* Send low-water mark */
#define SO_RCVTIMEO     20       /* Receive timeout */
#define SO_SNDTIMEO     21       /* Send timeout */
#define SO_ACCEPTCONN   30       /* Socket has had listen() */
#define SO_BINDTODEVICE 25       /* Bind to specific device */

/* Socket Options - SOL_TCP */
#define TCP_NODELAY     1        /* Don't delay send to coalesce packets */
#define TCP_MAXSEG      2        /* Set maximum segment size */
#define TCP_CORK        3        /* Never send partially complete segments */
#define TCP_KEEPIDLE    4        /* Start keeplives after this period */
#define TCP_KEEPINTVL   5        /* Interval between keepalives */
#define TCP_KEEPCNT     6        /* Number of keepalives before death */

/* send() and recv() flags */
#define MSG_OOB         0x01     /* Process out-of-band data */
#define MSG_PEEK        0x02     /* Peek at incoming message */
#define MSG_DONTROUTE   0x04     /* Don't use routing */
#define MSG_CTRUNC      0x08     /* Control data lost before delivery */
#define MSG_PROXY       0x10     /* Supply or ask for second address */
#define MSG_TRUNC       0x20     /* Data was truncated */
#define MSG_DONTWAIT    0x40     /* Non-blocking operation */
#define MSG_EOR         0x80     /* End of record */
#define MSG_WAITALL     0x100    /* Wait for full request or error */
#define MSG_NOSIGNAL    0x4000   /* Don't generate SIGPIPE */

/* shutdown() how values */
#define SHUT_RD         0        /* Shut down reading */
#define SHUT_WR         1        /* Shut down writing */
#define SHUT_RDWR       2        /* Shut down reading and writing */

/* Maximum queue length for listen() */
#define SOMAXCONN       128

#pragma pack(push, 1)

/* Generic socket address structure */
typedef struct sockaddr {
    uint16_t sa_family;          /* Address family */
    char     sa_data[14];        /* Address data */
} sockaddr_t;

/* IPv4 socket address structure */
typedef struct sockaddr_in {
    uint16_t sin_family;         /* Address family (AF_INET) */
    uint16_t sin_port;           /* Port number (network byte order) */
    uint32_t sin_addr;           /* IP address (network byte order) */
    uint8_t  sin_zero[8];        /* Padding to match sockaddr size */
} sockaddr_in_t;

/* Socket linger structure */
typedef struct linger {
    int l_onoff;                 /* Linger active */
    int l_linger;                /* How long to linger for */
} linger_t;

#pragma pack(pop)

/* Forward declarations */
struct socket;
struct msghdr;
struct iovec;

/* Socket API Functions */

/* Core socket operations */
int socket(int domain, int type, int protocol);
int bind(int sockfd, const struct sockaddr* addr, uint32_t addrlen);
int listen(int sockfd, int backlog);
int accept(int sockfd, struct sockaddr* addr, uint32_t* addrlen);
int connect(int sockfd, const struct sockaddr* addr, uint32_t addrlen);
int shutdown(int sockfd, int how);
int close(int sockfd);

/* Data transmission */
ssize_t send(int sockfd, const void* buf, size_t len, int flags);
ssize_t recv(int sockfd, void* buf, size_t len, int flags);
ssize_t sendto(int sockfd, const void* buf, size_t len, int flags,
               const struct sockaddr* dest_addr, uint32_t addrlen);
ssize_t recvfrom(int sockfd, void* buf, size_t len, int flags,
                 struct sockaddr* src_addr, uint32_t* addrlen);
ssize_t sendmsg(int sockfd, const struct msghdr* msg, int flags);
ssize_t recvmsg(int sockfd, struct msghdr* msg, int flags);

/* Socket options */
int setsockopt(int sockfd, int level, int optname,
               const void* optval, uint32_t optlen);
int getsockopt(int sockfd, int level, int optname,
               void* optval, uint32_t* optlen);

/* Socket information */
int getsockname(int sockfd, struct sockaddr* addr, uint32_t* addrlen);
int getpeername(int sockfd, struct sockaddr* addr, uint32_t* addrlen);

/* I/O multiplexing */
int select(int nfds, fd_set* readfds, fd_set* writefds,
           fd_set* exceptfds, struct timeval* timeout);
int poll(struct pollfd* fds, uint32_t nfds, int timeout);

/* Internal kernel socket API (exported for kernel modules) */
int sock_create_kern(int family, int type, int protocol, struct socket** res);
void sock_release(struct socket* sock);
int kernel_sendmsg(struct socket* sock, struct msghdr* msg, struct iovec* vec,
                   size_t num, size_t size);
int kernel_recvmsg(struct socket* sock, struct msghdr* msg, struct iovec* vec,
                   size_t num, size_t size, int flags);

#ifdef __cplusplus
}
#endif
