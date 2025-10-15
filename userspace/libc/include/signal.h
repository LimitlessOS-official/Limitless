#ifndef _SIGNAL_H
#define _SIGNAL_H

// Signal numbers (POSIX)
#define SIGHUP     1   // Hangup
#define SIGINT     2   // Interrupt
#define SIGQUIT    3   // Quit
#define SIGILL     4   // Illegal instruction
#define SIGTRAP    5   // Trace trap
#define SIGABRT    6   // Abort
#define SIGBUS     7   // Bus error
#define SIGFPE     8   // Floating point exception
#define SIGKILL    9   // Kill (cannot be caught)
#define SIGUSR1    10  // User-defined signal 1
#define SIGSEGV    11  // Segmentation violation
#define SIGUSR2    12  // User-defined signal 2
#define SIGPIPE    13  // Broken pipe
#define SIGALRM    14  // Alarm clock
#define SIGTERM    15  // Termination
#define SIGSTKFLT  16  // Stack fault
#define SIGCHLD    17  // Child stopped or terminated
#define SIGCONT    18  // Continue if stopped
#define SIGSTOP    19  // Stop (cannot be caught)
#define SIGTSTP    20  // Stop typed at terminal
#define SIGTTIN    21  // Background read from tty
#define SIGTTOU    22  // Background write to tty
#define SIGURG     23  // Urgent condition on socket
#define SIGXCPU    24  // CPU time limit exceeded
#define SIGXFSZ    25  // File size limit exceeded
#define SIGVTALRM  26  // Virtual alarm clock
#define SIGPROF    27  // Profiling alarm clock
#define SIGWINCH   28  // Window size change
#define SIGIO      29  // I/O now possible
#define SIGPWR     30  // Power failure
#define SIGSYS     31  // Bad system call

#define NSIG       32  // Number of signals

// Signal handler type
typedef void (*sighandler_t)(int);

// Special signal handlers
#define SIG_DFL ((sighandler_t)0)   // Default action
#define SIG_IGN ((sighandler_t)1)   // Ignore signal
#define SIG_ERR ((sighandler_t)-1)  // Error return

// Signal set type
typedef unsigned int sigset_t;

// Signal action structure
struct sigaction {
    sighandler_t sa_handler;
    sigset_t sa_mask;
    int sa_flags;
    void (*sa_restorer)(void);
};

// Signal functions
sighandler_t signal(int signum, sighandler_t handler);
int kill(int pid, int sig);
unsigned int alarm(unsigned int seconds);
int pause(void);
int sigprocmask(int how, const sigset_t *set, sigset_t *oldset);
int sigpending(sigset_t *set);
int sigaction(int signum, const struct sigaction *act, struct sigaction *oldact);

// Signal mask operations
#define SIG_BLOCK   0
#define SIG_UNBLOCK 1
#define SIG_SETMASK 2

// Signal set macros
#define sigemptyset(set)    (*(set) = 0)
#define sigfillset(set)     (*(set) = 0xFFFFFFFF)
#define sigaddset(set, sig) (*(set) |= (1U << ((sig) - 1)))
#define sigdelset(set, sig) (*(set) &= ~(1U << ((sig) - 1)))
#define sigismember(set, sig) ((*(set) & (1U << ((sig) - 1))) != 0)

#endif // _SIGNAL_H
