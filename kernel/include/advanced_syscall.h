/*
 * LimitlessOS Advanced System Call Framework Header
 * Enterprise-grade system call interface with comprehensive features
 */

#ifndef ADVANCED_SYSCALL_H
#define ADVANCED_SYSCALL_H

#include <stdint.h>
#include <stdbool.h>

/* System call categories */
typedef enum {
    SYSCALL_CAT_PROCESS = 0,    /* Process management */
    SYSCALL_CAT_MEMORY = 1,     /* Memory management */
    SYSCALL_CAT_FILE = 2,       /* File operations */
    SYSCALL_CAT_NETWORK = 3,    /* Network operations */
    SYSCALL_CAT_IPC = 4,        /* Inter-process communication */
    SYSCALL_CAT_TIME = 5,       /* Time and timers */
    SYSCALL_CAT_SIGNAL = 6,     /* Signal handling */
    SYSCALL_CAT_SYSTEM = 7,     /* System information */
    SYSCALL_CAT_SECURITY = 8,   /* Security operations */
    SYSCALL_CAT_ENTERPRISE = 9, /* Enterprise features */
    SYSCALL_CAT_MAX = 10
} syscall_category_t;

/* System call security levels */
typedef enum {
    SYSCALL_SEC_PUBLIC = 0,     /* Public, unrestricted */
    SYSCALL_SEC_USER = 1,       /* Requires user privileges */
    SYSCALL_SEC_ADMIN = 2,      /* Requires admin privileges */
    SYSCALL_SEC_SYSTEM = 3,     /* System-level access */
    SYSCALL_SEC_KERNEL = 4,     /* Kernel-level access */
    SYSCALL_SEC_ENTERPRISE = 5  /* Enterprise security context */
} syscall_security_level_t;

/* Forward declarations */
typedef struct syscall_descriptor syscall_descriptor_t;
typedef struct syscall_trace_entry syscall_trace_entry_t;
typedef struct syscall_audit_entry syscall_audit_entry_t;

/* ============================================================================
 * CORE SYSTEM CALL API
 * ============================================================================ */

/**
 * Initialize advanced system call framework
 * @return 0 on success, negative on error
 */
int advanced_syscall_init(void);

/**
 * Register a system call
 * @param number System call number
 * @param name System call name
 * @param category System call category
 * @param sec_level Security level required
 * @param arg_count Number of arguments
 * @param handler Function pointer to system call implementation
 * @return 0 on success, negative on error
 */
int syscall_register(uint32_t number, const char* name, syscall_category_t category,
                    syscall_security_level_t sec_level, uint32_t arg_count,
                    int (*handler)(uint64_t* args));

/**
 * Dispatch a system call
 * @param syscall_num System call number
 * @param args Array of system call arguments
 * @return System call return value
 */
int syscall_dispatch(uint32_t syscall_num, uint64_t* args);

/**
 * Unregister a system call
 * @param number System call number to unregister
 * @return 0 on success, negative on error
 */
int syscall_unregister(uint32_t number);

/* ============================================================================
 * TRACING API
 * ============================================================================ */

/**
 * Enable/disable system call tracing
 * @param enable true to enable, false to disable
 */
void syscall_enable_tracing(bool enable);

/**
 * Get trace entries
 * @param buffer Output buffer for trace entries
 * @param max_entries Maximum number of entries to return
 * @return Number of entries copied
 */
int syscall_get_trace_entries(syscall_trace_entry_t* buffer, uint32_t max_entries);

/**
 * Clear trace buffer
 */
void syscall_clear_trace_buffer(void);

/**
 * Configure trace filtering
 * @param category_mask Bitmask of categories to trace
 * @param min_duration_ns Minimum duration to trace (0 for all)
 * @return 0 on success, negative on error
 */
int syscall_configure_trace_filter(uint32_t category_mask, uint64_t min_duration_ns);

/* ============================================================================
 * AUDITING API
 * ============================================================================ */

/**
 * Enable/disable system call auditing
 * @param enable true to enable, false to disable
 */
void syscall_enable_auditing(bool enable);

/**
 * Get audit entries
 * @param buffer Output buffer for audit entries
 * @param max_entries Maximum number of entries to return
 * @return Number of entries copied
 */
int syscall_get_audit_entries(syscall_audit_entry_t* buffer, uint32_t max_entries);

/**
 * Clear audit buffer
 */
void syscall_clear_audit_buffer(void);

/**
 * Set audit severity threshold
 * @param min_severity Minimum severity level to audit (1-5)
 * @return 0 on success, negative on error
 */
int syscall_set_audit_threshold(uint32_t min_severity);

/* ============================================================================
 * RATE LIMITING API
 * ============================================================================ */

/**
 * Enable/disable rate limiting
 * @param enable true to enable, false to disable
 */
void syscall_enable_rate_limiting(bool enable);

/**
 * Set rate limit for a system call
 * @param syscall_num System call number
 * @param calls_per_second Maximum calls per second
 * @param burst_size Burst bucket size
 * @return 0 on success, negative on error
 */
int syscall_set_rate_limit(uint32_t syscall_num, uint32_t calls_per_second, uint32_t burst_size);

/**
 * Set global rate limit for a process
 * @param pid Process ID (0 for all processes)
 * @param calls_per_second Maximum calls per second
 * @return 0 on success, negative on error
 */
int syscall_set_process_rate_limit(uint32_t pid, uint32_t calls_per_second);

/**
 * Get rate limiting statistics
 * @param total_limited Output for total rate limited calls
 * @param processes_affected Output for number of processes affected
 * @return 0 on success, negative on error
 */
int syscall_get_rate_limit_stats(uint64_t* total_limited, uint32_t* processes_affected);

/* ============================================================================
 * SANDBOXING API
 * ============================================================================ */

/**
 * Enable/disable sandboxing
 * @param enable true to enable, false to disable
 */
void syscall_enable_sandboxing(bool enable);

/**
 * Add sandbox rule
 * @param rule_name Name of the sandbox rule
 * @param category System call category to restrict
 * @param syscall_mask Bitmask of allowed system calls
 * @param uid_min Minimum UID for rule application
 * @param uid_max Maximum UID for rule application
 * @return Rule ID on success, negative on error
 */
int syscall_add_sandbox_rule(const char* rule_name, syscall_category_t category,
                             uint32_t syscall_mask, uint32_t uid_min, uint32_t uid_max);

/**
 * Remove sandbox rule
 * @param rule_id Rule ID to remove
 * @return 0 on success, negative on error
 */
int syscall_remove_sandbox_rule(uint32_t rule_id);

/**
 * Apply sandbox rule to process
 * @param pid Process ID
 * @param rule_id Sandbox rule ID
 * @return 0 on success, negative on error
 */
int syscall_apply_sandbox_rule(uint32_t pid, uint32_t rule_id);

/**
 * Get sandbox violation statistics
 * @param total_violations Output for total violations
 * @param blocked_calls Output for total blocked calls
 * @return 0 on success, negative on error
 */
int syscall_get_sandbox_stats(uint64_t* total_violations, uint64_t* blocked_calls);

/* ============================================================================
 * SECURITY AND MONITORING API
 * ============================================================================ */

/**
 * Set system call security level
 * @param syscall_num System call number
 * @param sec_level New security level
 * @return 0 on success, negative on error
 */
int syscall_set_security_level(uint32_t syscall_num, syscall_security_level_t sec_level);

/**
 * Enable monitoring for system call
 * @param syscall_num System call number
 * @param enable true to enable, false to disable
 * @return 0 on success, negative on error
 */
int syscall_enable_monitoring(uint32_t syscall_num, bool enable);

/**
 * Get security violation statistics
 * @param privilege_escalations Output for privilege escalation attempts
 * @param unauthorized_access Output for unauthorized access attempts
 * @param security_violations Output for total security violations
 * @return 0 on success, negative on error
 */
int syscall_get_security_stats(uint64_t* privilege_escalations, uint64_t* unauthorized_access,
                               uint64_t* security_violations);

/**
 * Set enterprise security policy
 * @param policy_id Policy identifier
 * @param enabled true to enable policy, false to disable
 * @return 0 on success, negative on error
 */
int syscall_set_enterprise_policy(uint32_t policy_id, bool enabled);

/* ============================================================================
 * PERFORMANCE MONITORING API
 * ============================================================================ */

/**
 * Get system call performance statistics
 * @param syscall_num System call number (0 for all)
 * @param call_count Output for total call count
 * @param avg_time_ns Output for average execution time
 * @param max_time_ns Output for maximum execution time
 * @return 0 on success, negative on error
 */
int syscall_get_performance_stats(uint32_t syscall_num, uint64_t* call_count,
                                 uint64_t* avg_time_ns, uint64_t* max_time_ns);

/**
 * Set performance monitoring threshold
 * @param syscall_num System call number (0 for all)
 * @param threshold_ns Threshold in nanoseconds for slow call detection
 * @return 0 on success, negative on error
 */
int syscall_set_performance_threshold(uint32_t syscall_num, uint64_t threshold_ns);

/**
 * Get slow system call statistics
 * @param slow_calls Output for number of slow calls detected
 * @param slowest_syscall Output for syscall number of slowest call
 * @param slowest_time_ns Output for execution time of slowest call
 * @return 0 on success, negative on error
 */
int syscall_get_slow_call_stats(uint64_t* slow_calls, uint32_t* slowest_syscall,
                                uint64_t* slowest_time_ns);

/* ============================================================================
 * ENTERPRISE FEATURES API
 * ============================================================================ */

/**
 * Enable enterprise mode
 * @param enable true to enable, false to disable
 */
void syscall_enable_enterprise_mode(bool enable);

/**
 * Set tenant isolation
 * @param tenant_id Tenant identifier
 * @param isolation_level Isolation level (0-10)
 * @return 0 on success, negative on error
 */
int syscall_set_tenant_isolation(uint32_t tenant_id, uint32_t isolation_level);

/**
 * Enable compliance logging
 * @param compliance_standard Compliance standard ID
 * @param enable true to enable, false to disable
 * @return 0 on success, negative on error
 */
int syscall_enable_compliance_logging(uint32_t compliance_standard, bool enable);

/**
 * Get enterprise statistics
 * @param enterprise_calls Output for total enterprise calls
 * @param compliance_checks Output for compliance checks performed
 * @param violations Output for compliance violations
 * @return 0 on success, negative on error
 */
int syscall_get_enterprise_stats(uint64_t* enterprise_calls, uint64_t* compliance_checks,
                                uint64_t* violations);

/* ============================================================================
 * UTILITY AND MANAGEMENT API
 * ============================================================================ */

/**
 * Get list of registered system calls
 * @param syscall_list Output buffer for system call numbers
 * @param max_syscalls Maximum number of syscalls to return
 * @return Number of system calls copied
 */
int syscall_get_registered_list(uint32_t* syscall_list, uint32_t max_syscalls);

/**
 * Get system call information
 * @param syscall_num System call number
 * @param name Output buffer for system call name (64 bytes)
 * @param category Output for system call category
 * @param sec_level Output for security level
 * @return 0 on success, negative on error
 */
int syscall_get_info(uint32_t syscall_num, char* name, syscall_category_t* category,
                    syscall_security_level_t* sec_level);

/**
 * Print comprehensive system call statistics
 */
void syscall_print_statistics(void);

/**
 * System call framework self-test
 * @return 0 on success, negative on test failure
 */
int syscall_self_test(void);

/**
 * Shutdown system call framework
 */
void syscall_shutdown(void);

/* ============================================================================
 * STRUCTURE DEFINITIONS
 * ============================================================================ */

/* System call trace entry structure */
struct syscall_trace_entry {
    uint64_t timestamp;
    uint32_t pid;
    uint32_t tid;
    uint32_t syscall_num;
    uint64_t args[8];           /* System call arguments */
    int result;
    uint64_t duration_ns;
    uint32_t flags;
    
    /* Security context */
    uint32_t uid;
    uint32_t gid;
    uint32_t security_label;
    char comm[16];              /* Process name */
};

/* System call audit entry structure */
struct syscall_audit_entry {
    uint64_t timestamp;
    uint32_t pid;
    uint32_t uid;
    uint32_t syscall_num;
    char syscall_name[64];
    int result;
    uint32_t severity;          /* Audit severity level */
    char description[256];      /* Audit description */
    
    /* Enterprise context */
    uint32_t tenant_id;
    uint32_t session_id;
    char user_name[32];
    char host_name[64];
};

#endif /* ADVANCED_SYSCALL_H */