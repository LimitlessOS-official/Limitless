#ifndef NAMESPACE_H
#define NAMESPACE_H

#include <stdint.h>
#include <stdbool.h>

#define MAX_NAMESPACES 128

// Forward declarations
struct process; 
struct nsproxy;

// Namespace types
typedef enum {
    NS_TYPE_PID,
    NS_TYPE_MNT,
    NS_TYPE_NET,
    NS_TYPE_UTS,
    NS_TYPE_IPC,
    NS_TYPE_USER,
    NS_TYPE_COUNT // Number of namespace types
} ns_type_t;

// Generic namespace structure
typedef struct ns_common {
    int refcount;
    ns_type_t type;
} ns_common_t;

// PID Namespace
typedef struct pid_namespace {
    ns_common_t common;
    // PID-specific data would go here, e.g., PID tree, last_pid
} pid_namespace_t;

// Mount Namespace
typedef struct mnt_namespace {
    ns_common_t common;
    // Mount-specific data, e.g., pointer to root dentry
} mnt_namespace_t;

// Network Namespace
typedef struct net_namespace {
    ns_common_t common;
    // Network-specific data, e.g., loopback device, netdev list
} net_namespace_t;

// UTS Namespace
typedef struct uts_namespace {
    ns_common_t common;
    char hostname[64];
    char domainname[64];
} uts_namespace_t;

// IPC Namespace
typedef struct ipc_namespace {
    ns_common_t common;
    // IPC-specific data, e.g., message queues, semaphores
} ipc_namespace_t;

// User Namespace
typedef struct user_namespace {
    ns_common_t common;
    // User-specific data, e.g., UID/GID mappings
} user_namespace_t;


// Namespace Proxy: A collection of pointers to a process's namespaces
typedef struct nsproxy {
    int refcount;
    uts_namespace_t *uts_ns;
    ipc_namespace_t *ipc_ns;
    mnt_namespace_t *mnt_ns;
    pid_namespace_t *pid_ns;
    net_namespace_t *net_ns;
    user_namespace_t *user_ns;
} nsproxy_t;


// Public Function Prototypes
void namespaces_init(void);
struct nsproxy* create_nsproxy(struct process* p);
void switch_namespace(struct process* p, struct nsproxy* new_ns);
void free_nsproxy(struct nsproxy* ns);

// Functions to create new namespaces
nsproxy_t* copy_nsproxy(nsproxy_t* old_ns);
int unshare_nsproxy(unsigned long unshare_flags, nsproxy_t** nsproxy);
int switch_nsproxy(nsproxy_t* new_ns);
nsproxy_t* get_init_nsproxy(void);


#endif // NAMESPACE_H
