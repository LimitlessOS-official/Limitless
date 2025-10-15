
#pragma once
#include "kernel.h"

/* Forward declaration to avoid circular dependency */
typedef struct process process_t;

/* POSIX Signal Definitions */
#define SIGHUP      1   /* Hangup */
#define SIGINT      2   /* Interrupt */
#define SIGQUIT     3   /* Quit */
#define SIGILL      4   /* Illegal instruction */
#define SIGTRAP     5   /* Trace/breakpoint trap */
#define SIGABRT     6   /* Abort */
#define SIGBUS      7   /* Bus error */
#define SIGFPE      8   /* Floating point exception */
#define SIGKILL     9   /* Kill (cannot be caught or ignored) */
#define SIGUSR1     10  /* User-defined signal 1 */
#define SIGSEGV     11  /* Segmentation violation */
#define SIGUSR2     12  /* User-defined signal 2 */
#define SIGPIPE     13  /* Broken pipe */
#define SIGALRM     14  /* Alarm clock */
#define SIGTERM     15  /* Termination */
#define SIGSTKFLT   16  /* Stack fault */
#define SIGCHLD     17  /* Child status changed */
#define SIGCONT     18  /* Continue (cannot be blocked) */
#define SIGSTOP     19  /* Stop (cannot be caught or ignored) */
#define SIGTSTP     20  /* Keyboard stop */
#define SIGTTIN     21  /* Background read from tty */
#define SIGTTOU     22  /* Background write to tty */
#define SIGURG      23  /* Urgent condition on socket */
#define SIGXCPU     24  /* CPU time limit exceeded */
#define SIGXFSZ     25  /* File size limit exceeded */
#define SIGVTALRM   26  /* Virtual alarm clock */
#define SIGPROF     27  /* Profiling alarm clock */
#define SIGWINCH    28  /* Window size change */
#define SIGIO       29  /* I/O now possible */
#define SIGPWR      30  /* Power failure restart */
#define SIGSYS      31  /* Bad system call */

#define _NSIG       64  /* Number of signals */

/* Signal actions */
#define SIG_DFL     ((sighandler_t) 0)   /* Default action */
#define SIG_IGN     ((sighandler_t) 1)   /* Ignore signal */
#define SIG_ERR     ((sighandler_t) -1)  /* Error return */

/* Signal action flags */
#define SA_NOCLDSTOP  0x00000001
#define SA_NOCLDWAIT  0x00000002
#define SA_SIGINFO    0x00000004
#define SA_RESTORER   0x04000000
#define SA_ONSTACK    0x08000000
#define SA_RESTART    0x10000000
#define SA_INTERRUPT  0x20000000
#define SA_NODEFER    0x40000000
#define SA_RESETHAND  0x80000000

/* Signal mask operations */
#define SIG_BLOCK     0  /* Block signals */
#define SIG_UNBLOCK   1  /* Unblock signals */
#define SIG_SETMASK   2  /* Set signal mask */

/* Signal mask (64 bits for 64 signals) */
typedef uint64_t sigset_t;

/* Signal value union */
union sigval {
    int sival_int;
    void* sival_ptr;
};

/* Signal information structure */
typedef struct siginfo {
    int si_signo;    /* Signal number */
    int si_errno;    /* Error number */
    int si_code;     /* Signal code */
    pid_t si_pid;    /* Sending process ID */
    uid_t si_uid;    /* Sending process real user ID */
    int si_status;   /* Exit value or signal */
    void* si_addr;   /* Memory location which caused fault */
    long si_band;    /* Band event for SIGPOLL */
    union {
        int _pad[29];
        struct {
            pid_t _pid;
            uid_t _uid;
        } _kill;
        struct {
            int _tid;
            int _overrun;
            union sigval _sigval;
        } _timer;
        struct {
            pid_t _pid;
            uid_t _uid;
            union sigval _sigval;
        } _rt;
        struct {
            pid_t _pid;
            uid_t _uid;
            int _status;
            long _utime;
            long _stime;
        } _sigchld;
        struct {
            void* _addr;
        } _sigfault;
        struct {
            long _band;
            int _fd;
        } _sigpoll;
    } _sifields;
} siginfo_t;

/* Signal handler types */
typedef void (*sighandler_t)(int);
typedef void (*sigaction_t)(int, siginfo_t*, void*);

/* Signal action structure */
typedef struct sigaction {
    union {
        sighandler_t sa_handler;
        sigaction_t sa_sigaction;
    };
    sigset_t sa_mask;
    int sa_flags;
    void (*sa_restorer)(void);
} signal_action_t;

/* Pending signal information */
typedef struct pending_signal {
    int signo;
    siginfo_t info;
    struct pending_signal* next;
} pending_signal_t;

/* Per-process signal state */
typedef struct signal_state {
    signal_action_t actions[_NSIG];      /* Signal handlers */
    sigset_t blocked;                    /* Blocked signals mask */
    sigset_t pending;                    /* Pending signals mask */
    pending_signal_t* pending_queue;     /* Pending signal queue */
    uint32_t pending_count;              /* Number of pending signals */
    
    /* Signal stack */
    void* alt_stack;                     /* Alternative signal stack */
    size_t alt_stack_size;              /* Alternative stack size */
    int alt_stack_flags;                /* Stack flags */
    
    /* Signal delivery context */
    bool in_signal_handler;             /* Currently in signal handler */
    sigset_t saved_mask;                /* Saved signal mask */
    void* signal_frame;                 /* Signal frame on user stack */
    
    spinlock_t lock;                    /* Signal state lock */
} signal_state_t;

/* Signal frame for user-space signal handlers */
typedef struct signal_frame {
    uint64_t rdi, rsi, rdx, rcx, r8, r9;  /* Function call arguments */
    uint64_t rax, rbx, rbp, r10, r11, r12, r13, r14, r15;
    uint64_t rip, rsp;                    /* Instruction and stack pointers */
    uint64_t rflags;                      /* Flags register */
    siginfo_t siginfo;                    /* Signal information */
    signal_action_t* action;              /* Signal action */
    sigset_t old_mask;                    /* Previous signal mask */
    void* restorer;                       /* Signal return address */
} signal_frame_t;

/* Core signal functions */
void signal_init(signal_state_t* sigstate);
void signal_cleanup(signal_state_t* sigstate);
int signal_send(process_t* target, int signo, siginfo_t* info);
void signal_deliver_pending(process_t* proc);

/* System call implementations */
int sys_sigaction(int signo, const signal_action_t* act, signal_action_t* oldact);
int sys_sigprocmask(int how, const sigset_t* set, sigset_t* oldset);
int sys_kill(pid_t pid, int signo);
int sys_sigreturn(void);

/* Signal utility functions */
int sigemptyset(sigset_t* set);
int sigfillset(sigset_t* set);
int sigaddset(sigset_t* set, int signo);
int sigdelset(sigset_t* set, int signo);
int sigismember(const sigset_t* set, int signo);
