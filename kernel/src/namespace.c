#include "namespace.h"
#include <stddef.h> // For NULL
#include <string.h> // For memcpy and strcpy

// A simple slab allocator would be better, but for now, a static array will do.
static nsproxy_t nsproxy_pool[MAX_NAMESPACES];
static int next_nsproxy = 0;

static uts_namespace_t uts_ns_pool[MAX_NAMESPACES];
static int next_uts_ns = 0;

// Initial, root namespace proxy
static nsproxy_t init_nsproxy;
static uts_namespace_t init_uts_ns;

/**
 * @brief In a real kernel, this would be defined in a process management header.
 * We define a placeholder here for simulation purposes.
 */
struct task_struct {
    nsproxy_t *nsproxy;
};

// The "current" running process/task. In a real kernel, this is a global
// or per-CPU variable updated by the scheduler.
static struct task_struct *current_task = NULL;

/**
 * @brief Initializes the entire namespace subsystem.
 */
void namespaces_init(void) {
    // Initialize the root UTS namespace
    init_uts_ns.common.refcount = 1;
    init_uts_ns.common.type = NS_TYPE_UTS;
    strcpy(init_uts_ns.hostname, "limitless");
    strcpy(init_uts_ns.domainname, "localdomain");

    // Initialize the root namespace proxy
    init_nsproxy.refcount = 1;
    init_nsproxy.uts_ns = &init_uts_ns;
    // In a full implementation, other initial namespaces (pid, mnt, etc.) would be created here.
    init_nsproxy.ipc_ns = NULL;
    init_nsproxy.mnt_ns = NULL;
    init_nsproxy.pid_ns = NULL;
    init_nsproxy.net_ns = NULL;
    init_nsproxy.user_ns = NULL;

    // Clear the pools
    memset(nsproxy_pool, 0, sizeof(nsproxy_pool));
    memset(uts_ns_pool, 0, sizeof(uts_ns_pool));
    next_nsproxy = 0;
    next_uts_ns = 0;

    // Simulate an initial task
    static struct task_struct initial_task;
    initial_task.nsproxy = &init_nsproxy;
    current_task = &initial_task;
}

/**
 * @brief Creates a new namespace proxy, typically for a new process.
 *        Initially, it just points to the initial namespaces.
 * @param p The process for which to create the nsproxy.
 * @return A pointer to the new nsproxy, or NULL on failure.
 */
struct nsproxy* create_nsproxy(struct process* p) {
    // For now, all new processes share the initial nsproxy.
    // A real implementation would clone the parent's nsproxy.
    init_nsproxy.refcount++;
    return &init_nsproxy;
}

/**
 * @brief Frees a namespace proxy, decrementing reference counts.
 * @param ns The nsproxy to free.
 */
void free_nsproxy(struct nsproxy* ns) {
    if (ns) {
        ns->refcount--;
        if (ns->refcount == 0) {
            // In a real implementation, free each contained namespace if its refcount hits 0.
            // For this simulation, we only handle the UTS namespace.
            if (ns->uts_ns) {
                ns->uts_ns->common.refcount--;
            }
            // ... free other namespaces ...
        }
    }
}

/**
 * @brief Copies an existing nsproxy, incrementing refcounts.
 * @param old_ns The nsproxy to copy.
 * @return A pointer to a new nsproxy in the pool, or NULL on failure.
 */
nsproxy_t* copy_nsproxy(nsproxy_t* old_ns) {
    if (next_nsproxy >= MAX_NAMESPACES) {
        return NULL; // Pool exhausted
    }

    nsproxy_t* new_ns = &nsproxy_pool[next_nsproxy++];
    memcpy(new_ns, old_ns, sizeof(nsproxy_t));
    new_ns->refcount = 1;

    // Increment refcounts of all pointed-to namespaces
    if (new_ns->uts_ns) new_ns->uts_ns->common.refcount++;
    // ... increment other namespace refcounts ...

    return new_ns;
}

/**
 * @brief Creates new namespaces for a process based on unshare flags.
 * @param unshare_flags A bitmask of flags indicating which namespaces to unshare.
 * @param nsproxy_ptr A pointer to the process's nsproxy pointer.
 * @return 0 on success, -1 on failure.
 */
int unshare_nsproxy(unsigned long unshare_flags, nsproxy_t** nsproxy_ptr) {
    nsproxy_t* current_ns = *nsproxy_ptr;
    nsproxy_t* new_ns = copy_nsproxy(current_ns);
    if (!new_ns) {
        return -1; // Failed to copy
    }

    // If we are creating a new UTS namespace
    if (unshare_flags & 0x04000000) { // CLONE_NEWUTS
        if (next_uts_ns >= MAX_NAMESPACES) {
            free_nsproxy(new_ns);
            return -1; // Pool exhausted
        }
        uts_namespace_t* new_uts = &uts_ns_pool[next_uts_ns++];
        
        // Copy the data from the old UTS namespace
        memcpy(new_uts, current_ns->uts_ns, sizeof(uts_namespace_t));
        new_uts->common.refcount = 1;

        // Point the new nsproxy to the new UTS namespace
        new_ns->uts_ns = new_uts;
    }

    // ... handle other namespace types (CLONE_NEWPID, CLONE_NEWNET, etc.) ...

    // Switch the process to the new nsproxy
    *nsproxy_ptr = new_ns;
    
    // Decrement refcount of the old nsproxy
    free_nsproxy(current_ns);

    return 0;
}

/**
 * @brief Switches a process to a new set of namespaces.
 * @param p The process to modify.
 * @param new_ns The new nsproxy to assign.
 */
void switch_namespace(struct process* p, struct nsproxy* new_ns) {
    // In a real OS, this would be more complex, involving task_struct modification.
    // For now, we assume the process struct has a pointer to its nsproxy.
    // if (p->nsproxy) {
    //     free_nsproxy(p->nsproxy);
    // }
    // p->nsproxy = new_ns;
    // if (new_ns) {
    //     new_ns->refcount++;
    // }
}

/**
 * @brief Simulates the setns() syscall's core logic for the current task.
 * @param new_ns The nsproxy to switch to.
 * @return 0 on success, -1 on failure.
 */
int switch_nsproxy(nsproxy_t* new_ns) {
    if (!current_task || !new_ns) {
        return -1;
    }

    nsproxy_t* old_ns = current_task->nsproxy;

    // In a real kernel, there would be extensive permission checks here.
    // (e.g., capable(CAP_SYS_ADMIN) in the target user namespace).

    if (old_ns != new_ns) {
        // Assign the new namespace proxy
        current_task->nsproxy = new_ns;
        new_ns->refcount++;

        // Release the old namespace proxy
        free_nsproxy(old_ns);
    }

    return 0;
}

/**
 * @brief Returns the initial, root namespace proxy.
 * @return A pointer to init_nsproxy.
 */
nsproxy_t* get_init_nsproxy(void) {
    return &init_nsproxy;
}
