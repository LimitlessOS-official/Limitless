/*
 * LimitlessOS Advanced Signal System Header
 * Enterprise-grade signal handling with real-time signals, signal queuing,
 * advanced delivery mechanisms, per-process signal masks, and POSIX compliance
 */

#ifndef ADVANCED_SIGNAL_H
#define ADVANCED_SIGNAL_H

#include <stdint.h>
#include <stdbool.h>

/* ============================================================================
 * SIGNAL CONSTANTS AND DEFINITIONS
 * ============================================================================ */

/* Standard POSIX signals */
#define SIGHUP      1   /* Hangup */
#define SIGINT      2   /* Interrupt */
#define SIGQUIT     3   /* Quit */
#define SIGILL      4   /* Illegal instruction */
#define SIGTRAP     5   /* Trace trap */
#define SIGABRT     6   /* Abort */
#define SIGBUS      7   /* Bus error */
#define SIGFPE      8   /* Floating point exception */
#define SIGKILL     9   /* Kill (cannot be caught or ignored) */
#define SIGUSR1     10  /* User defined signal 1 */
#define SIGSEGV     11  /* Segmentation violation */
#define SIGUSR2     12  /* User defined signal 2 */
#define SIGPIPE     13  /* Broken pipe */
#define SIGALRM     14  /* Alarm clock */
#define SIGTERM     15  /* Termination */
#define SIGSTKFLT   16  /* Stack fault */
#define SIGCHLD     17  /* Child status changed */
#define SIGCONT     18  /* Continue */
#define SIGSTOP     19  /* Stop (cannot be caught or ignored) */
#define SIGTSTP     20  /* Keyboard stop */
#define SIGTTIN     21  /* Background read from tty */
#define SIGTTOU     22  /* Background write to tty */
#define SIGURG      23  /* Urgent condition on socket */
#define SIGXCPU     24  /* CPU limit exceeded */
#define SIGXFSZ     25  /* File size limit exceeded */
#define SIGVTALRM   26  /* Virtual alarm clock */
#define SIGPROF     27  /* Profiling alarm clock */
#define SIGWINCH    28  /* Window size change */
#define SIGIO       29  /* I/O now possible */
#define SIGPWR      30  /* Power failure restart */
#define SIGSYS      31  /* Bad system call */

/* Real-time signals (POSIX.1b) */
#define SIGRTMIN    32  /* Minimum real-time signal */
#define SIGRTMAX    63  /* Maximum real-time signal */

/* Enterprise-specific signals */
#define SIGPERF     64  /* Performance monitoring */
#define SIGAUDIT    65  /* Audit event */
#define SIGCONTAINER 66 /* Container event */
#define SIGCRYPTO   67  /* Cryptographic event */
#define SIGNET      68  /* Network event */
#define SIGFS       69  /* Filesystem event */
#define SIGMEM      70  /* Memory event */
#define SIGPOWER    71  /* Power management event */

#define MAX_SIGNALS 72

/* Signal action flags */
#define SA_NOCLDSTOP    0x00000001  /* Don't send SIGCHLD when children stop */
#define SA_NOCLDWAIT    0x00000002  /* Don't create zombie on child death */
#define SA_SIGINFO      0x00000004  /* Invoke signal-catching function with three arguments */
#define SA_ONSTACK      0x08000000  /* Execute signal handler on signal stack */
#define SA_RESTART      0x10000000  /* Restart functions if interrupted by handler */
#define SA_NODEFER      0x40000000  /* Don't automatically block the signal when executing handler */
#define SA_RESETHAND    0x80000000  /* Reset to SIG_DFL on entry to handler */

/* Signal mask operation types */
#define SIG_BLOCK       0   /* Block signals */
#define SIG_UNBLOCK     1   /* Unblock signals */
#define SIG_SETMASK     2   /* Set signal mask */

/* Special signal handler values */
#define SIG_DFL         ((sig_handler_t)0)      /* Default handler */
#define SIG_IGN         ((sig_handler_t)1)      /* Ignore signal */
#define SIG_ERR         ((sig_handler_t)-1)     /* Error return */

/* ============================================================================
 * DATA STRUCTURES
 * ============================================================================ */

/* Signal delivery mechanisms */
typedef enum {
    SIGNAL_DELIVERY_SYNCHRONOUS,    /* Immediate delivery to target thread */
    SIGNAL_DELIVERY_ASYNCHRONOUS,   /* Queue for later delivery */
    SIGNAL_DELIVERY_BROADCAST,      /* Deliver to all threads in process */
    SIGNAL_DELIVERY_ROUND_ROBIN,    /* Distribute across available threads */
    SIGNAL_DELIVERY_PRIORITY,       /* Deliver to highest priority thread */
    SIGNAL_DELIVERY_AFFINITY        /* Deliver based on CPU affinity */
} signal_delivery_t;

/* Signal handler types */
typedef enum {
    SIGNAL_HANDLER_DEFAULT,         /* Default kernel handler */
    SIGNAL_HANDLER_IGNORE,          /* Ignore the signal */
    SIGNAL_HANDLER_USER,           /* User-space handler function */
    SIGNAL_HANDLER_KERNEL,         /* Kernel-space handler function */
    SIGNAL_HANDLER_REAL_TIME       /* Real-time signal handler */
} signal_handler_type_t;

/* Signal information structure */
typedef struct siginfo {
    int si_signo;           /* Signal number */
    int si_errno;           /* Errno value */
    int si_code;            /* Signal code */
    
    union {
        /* SIGCHLD */
        struct {
            uint32_t si_pid;    /* Process ID */
            uint32_t si_uid;    /* User ID */
            int si_status;      /* Exit value or signal */
        } sigchld;
        
        /* SIGILL, SIGFPE, SIGSEGV, SIGBUS */
        struct {
            void* si_addr;      /* Faulting instruction address */
        } sigfault;
        
        /* SIGPOLL */
        struct {
            long si_band;       /* Band event for SIGPOLL */
            int si_fd;          /* File descriptor */
        } sigpoll;
        
        /* Real-time signals */
        struct {
            int si_int;         /* Integer value */
            void* si_ptr;       /* Pointer value */
        } sigrt;
        
        /* Enterprise signals */
        struct {
            uint64_t si_timestamp;  /* Event timestamp */
            uint32_t si_source;     /* Event source */
            uint64_t si_data[4];    /* Extended data */
        } sigent;
    };
} siginfo_t;

/* Signal handler function types */
typedef void (*sig_handler_t)(int sig);
typedef void (*siginfo_handler_t)(int sig, siginfo_t* info, void* context);

/* Signal action structure */
typedef struct sigaction {
    union {
        sig_handler_t sa_handler;           /* Simple signal handler */
        siginfo_handler_t sa_sigaction;     /* Extended signal handler */
    };
    uint64_t sa_mask;                       /* Signal mask during handler execution */
    uint32_t sa_flags;                      /* Signal flags */
    void (*sa_restorer)(void);              /* Signal restorer function */
} sigaction_t;

/* Signal statistics structure */
typedef struct signal_statistics {
    uint64_t signals_sent;                  /* Signals sent by process */
    uint64_t signals_received;              /* Signals received by process */
    uint64_t signals_blocked;               /* Signals blocked count */
    uint64_t signals_queued;                /* Signals currently queued */
    uint64_t handler_invocations;           /* Handler invocation count */
    uint64_t handler_crashes;               /* Handler crash count */
    uint64_t average_delivery_latency_ns;   /* Average delivery latency */
    uint64_t max_delivery_latency_ns;       /* Maximum delivery latency */
    uint64_t queue_overflows;               /* Signal queue overflow count */
    uint64_t rt_signals_sent;              /* Real-time signals sent */
    uint64_t rt_signals_received;          /* Real-time signals received */
} signal_statistics_t;

/* Signal set type for compatibility */
typedef uint64_t sigset_t;

/* ============================================================================
 * CORE SIGNAL API
 * ============================================================================ */

/**
 * Send signal to process
 * @param pid Process ID to send signal to
 * @param signo Signal number to send
 * @return 0 on success, negative on error
 */
int kill(uint32_t pid, int signo);

/**
 * Set signal action
 * @param signo Signal number
 * @param act New signal action (can be NULL)
 * @param oldact Previous signal action (can be NULL)
 * @return 0 on success, negative on error
 */
int sigaction(int signo, const sigaction_t* act, sigaction_t* oldact);

/**
 * Modify signal mask for current thread
 * @param how How to modify mask (SIG_BLOCK, SIG_UNBLOCK, SIG_SETMASK)
 * @param set Signal set to apply (can be NULL)
 * @param oldset Previous signal mask (can be NULL)
 * @return 0 on success, negative on error
 */
int sigprocmask(int how, const uint64_t* set, uint64_t* oldset);

/**
 * Suspend execution until signal is delivered
 * @param mask Temporary signal mask during suspension
 * @return Always returns -1 (interrupted by signal)
 */
int sigsuspend(const uint64_t* mask);

/**
 * Get set of pending signals
 * @param set Output for pending signal mask
 * @return 0 on success, negative on error
 */
int sigpending(uint64_t* set);

/* ============================================================================
 * REAL-TIME SIGNAL API
 * ============================================================================ */

/**
 * Send real-time signal with data
 * @param pid Process ID to send signal to
 * @param signo Real-time signal number (SIGRTMIN-SIGRTMAX)
 * @param info Signal information with data
 * @return 0 on success, negative on error
 */
int sigqueue(uint32_t pid, int signo, const siginfo_t* info);

/**
 * Wait for signal with timeout
 * @param set Signal set to wait for
 * @param info Output for signal information
 * @param timeout_ns Timeout in nanoseconds (0 = no timeout)
 * @return Signal number on success, negative on error/timeout
 */
int sigtimedwait(const uint64_t* set, siginfo_t* info, uint64_t timeout_ns);

/**
 * Wait for signal with immediate return
 * @param set Signal set to wait for
 * @param info Output for signal information
 * @return Signal number on success, 0 if no signal, negative on error
 */
int sigwaitinfo(const uint64_t* set, siginfo_t* info);

/* ============================================================================
 * ENTERPRISE SIGNAL FEATURES
 * ============================================================================ */

/**
 * Enable signal sandboxing for process
 * @param pid Process ID to enable sandboxing for
 * @param allowed_signals Mask of allowed signals
 * @return 0 on success, negative on error
 */
int signal_enable_sandboxing(uint32_t pid, uint64_t allowed_signals);

/**
 * Set signal routing policy
 * @param signo Signal number
 * @param policy Delivery policy
 * @param cpu_affinity CPU affinity mask
 * @return 0 on success, negative on error
 */
int signal_set_routing_policy(int signo, signal_delivery_t policy, uint32_t cpu_affinity);

/**
 * Get comprehensive signal statistics
 * @param pid Process ID (0 for current process)
 * @param stats Output for statistics
 * @return 0 on success, negative on error
 */
int signal_get_statistics(uint32_t pid, signal_statistics_t* stats);

/**
 * Set signal queue parameters
 * @param max_queue_depth Maximum signals in queue per thread
 * @param rt_priority_boost Priority boost for real-time signals
 * @return 0 on success, negative on error
 */
int signal_set_queue_params(uint32_t max_queue_depth, uint32_t rt_priority_boost);

/**
 * Enable signal auditing
 * @param enable true to enable auditing, false to disable
 * @return 0 on success, negative on error
 */
int signal_enable_auditing(bool enable);

/**
 * Set signal encryption for security
 * @param enable true to enable encryption, false to disable
 * @param key_id Encryption key ID
 * @return 0 on success, negative on error
 */
int signal_set_encryption(bool enable, uint32_t key_id);

/* ============================================================================
 * SIGNAL SET MANIPULATION
 * ============================================================================ */

/**
 * Initialize empty signal set
 * @param set Signal set to initialize
 * @return 0 on success, negative on error
 */
int sigemptyset(uint64_t* set);

/**
 * Initialize full signal set
 * @param set Signal set to initialize
 * @return 0 on success, negative on error
 */
int sigfillset(uint64_t* set);

/**
 * Add signal to set
 * @param set Signal set to modify
 * @param signo Signal number to add
 * @return 0 on success, negative on error
 */
int sigaddset(uint64_t* set, int signo);

/**
 * Remove signal from set
 * @param set Signal set to modify
 * @param signo Signal number to remove
 * @return 0 on success, negative on error
 */
int sigdelset(uint64_t* set, int signo);

/**
 * Test if signal is in set
 * @param set Signal set to test
 * @param signo Signal number to test
 * @return 1 if in set, 0 if not, negative on error
 */
int sigismember(const uint64_t* set, int signo);

/* ============================================================================
 * SIGNAL STACK MANAGEMENT
 * ============================================================================ */

/* Signal stack structure */
typedef struct sigaltstack {
    void* ss_sp;        /* Stack pointer */
    int ss_flags;       /* Stack flags */
    size_t ss_size;     /* Stack size */
} stack_t;

/* Stack flags */
#define SS_ONSTACK      0x1     /* Currently on signal stack */
#define SS_DISABLE      0x2     /* Disable signal stack */

/**
 * Set alternate signal stack
 * @param stack New signal stack
 * @param oldstack Previous signal stack (can be NULL)
 * @return 0 on success, negative on error
 */
int sigaltstack(const stack_t* stack, stack_t* oldstack);

/* ============================================================================
 * PERFORMANCE AND MONITORING
 * ============================================================================ */

/**
 * Print comprehensive signal system statistics
 */
void signal_print_statistics(void);

/**
 * Get signal handler execution time
 * @param signo Signal number
 * @param total_time_ns Output for total execution time
 * @param invocation_count Output for invocation count
 * @return 0 on success, negative on error
 */
int signal_get_handler_stats(int signo, uint64_t* total_time_ns, uint64_t* invocation_count);

/**
 * Set signal performance monitoring
 * @param enable true to enable monitoring, false to disable
 * @return 0 on success, negative on error
 */
int signal_set_performance_monitoring(bool enable);

/**
 * Get signal delivery latency statistics
 * @param signo Signal number (0 for all signals)
 * @param avg_latency_ns Output for average latency
 * @param max_latency_ns Output for maximum latency
 * @return 0 on success, negative on error
 */
int signal_get_latency_stats(int signo, uint64_t* avg_latency_ns, uint64_t* max_latency_ns);

/* ============================================================================
 * SYSTEM INTEGRATION
 * ============================================================================ */

/**
 * Initialize advanced signal system
 * @return 0 on success, negative on error
 */
int advanced_signal_init(void);

/**
 * Shutdown signal system
 */
void advanced_signal_shutdown(void);

/**
 * Process pending signals for current thread
 */
void signal_process_pending_signals(void);

/**
 * Signal system self-test
 * @return 0 on success, negative on test failure
 */
int signal_self_test(void);

/* ============================================================================
 * ENTERPRISE EXTENSIONS
 * ============================================================================ */

/**
 * Set signal priority class
 * @param signo Signal number
 * @param priority_class Priority class (0=lowest, 3=highest)
 * @return 0 on success, negative on error
 */
int signal_set_priority_class(int signo, int priority_class);

/**
 * Enable signal compression for large payloads
 * @param enable true to enable compression, false to disable
 * @return 0 on success, negative on error
 */
int signal_enable_compression(bool enable);

/**
 * Set signal rate limiting
 * @param signo Signal number (0 for all signals)
 * @param max_rate Maximum signals per second
 * @return 0 on success, negative on error
 */
int signal_set_rate_limiting(int signo, uint32_t max_rate);

/**
 * Get signal security context
 * @param pid Process ID
 * @param context Output for security context
 * @return 0 on success, negative on error
 */
int signal_get_security_context(uint32_t pid, void* context);

#endif /* ADVANCED_SIGNAL_H */