#ifndef CAPABILITY_H
#define CAPABILITY_H

#include <stdint.h>
#include <stdbool.h>

// --- Capability Definitions ---

// The full set of capabilities supported by the system.
// This is a more comprehensive list than the one previously in security_framework.c
typedef enum {
    CAP_CHOWN,            // Change file ownership
    CAP_DAC_OVERRIDE,     // Bypass file read, write, and execute permission checks
    CAP_DAC_READ_SEARCH,  // Bypass file read permission checks and directory read/search permission checks
    CAP_FOWNER,           // Bypass permission checks on operations that need ownership of the file
    CAP_FSETID,           // Don't clear set-user-ID and set-group-ID permission bits when a file is modified
    CAP_KILL,             // Bypass permission checks for sending signals
    CAP_SETGID,           // Make arbitrary manipulations of process GIDs and supplementary GID list
    CAP_SETUID,           // Make arbitrary manipulations of process UIDs
    CAP_SETPCAP,          // Set capabilities
    CAP_LINUX_IMMUTABLE,  // Set the FS_IMMUTABLE_FL and FS_APPEND_FL i-node flags
    CAP_NET_BIND_SERVICE, // Bind a socket to privileged ports (ports < 1024)
    CAP_NET_BROADCAST,    // (Unused)
    CAP_NET_ADMIN,        // Perform various network-related operations
    CAP_NET_RAW,          // Use RAW and PACKET sockets
    CAP_IPC_LOCK,         // Lock memory (mlock, mlockall, mmap, shmctl)
    CAP_IPC_OWNER,        // Bypass permission checks for operations on System V IPC objects
    CAP_SYS_MODULE,       // Load and unload kernel modules
    CAP_SYS_RAWIO,        // Perform I/O port operations (iopl, ioperm)
    CAP_SYS_CHROOT,       // Use chroot()
    CAP_SYS_PTRACE,       // Trace arbitrary processes using ptrace()
    CAP_SYS_PACCT,        // Use acct()
    CAP_SYS_ADMIN,        // Perform a range of system administration operations
    CAP_SYS_BOOT,         // Use reboot() and kexec_load()
    CAP_SYS_NICE,         // Raise process nice value, set nice values for arbitrary processes
    CAP_SYS_RESOURCE,     // Override resource limits
    CAP_SYS_TIME,         // Set system clock
    CAP_SYS_TTY_CONFIG,   // Use vhangup()
    CAP_MKNOD,            // Create special files using mknod()
    CAP_LEASE,            // Establish leases on arbitrary files
    CAP_AUDIT_WRITE,      // Write records to kernel auditing log
    CAP_AUDIT_CONTROL,    // Enable and disable kernel auditing, change auditing filter rules
    CAP_SETFCAP,          // Set file capabilities
    CAP_MAC_OVERRIDE,     // Override Mandatory Access Control
    CAP_MAC_ADMIN,        // Allow MAC configuration or state changes
    CAP_SYSLOG,           // Perform privileged syslog operations
    CAP_WAKE_ALARM,       // Trigger something that will wake up the system
    CAP_BLOCK_SUSPEND,    // Block system suspend
    CAP_AUDIT_READ,       // Read the audit log via a multicast netlink socket
    CAP_LAST_CAP          // Should be last entry
} security_capability_t;

// A bitmask representing a set of capabilities.
typedef uint64_t cap_mask_t;

// A set of capabilities for a process (or file)
typedef struct {
    cap_mask_t permitted;   // What the process can ever have
    cap_mask_t effective;   // What the process currently has
    cap_mask_t inheritable; // What is passed across an execve
} capability_set_t;


// --- Public API ---

// Initialize the capability subsystem
void capability_init(void);

// Get the capabilities of a security context
capability_set_t* capability_get(uint32_t context_id);

// Set the capabilities of a security context
int capability_set(uint32_t context_id, const capability_set_t* caps);

// Check if a security context has a specific capability in its effective set
bool has_capability(uint32_t context_id, security_capability_t cap);

// Raise a capability in the effective set (if it's in the permitted set)
int capability_raise(uint32_t context_id, security_capability_t cap);

// Lower a capability from the effective set
int capability_lower(uint32_t context_id, security_capability_t cap);

#endif // CAPABILITY_H
