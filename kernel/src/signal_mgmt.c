/*
 * Signal Handling Implementation
 * 
 * Complete signal system for inter-process communication
 * and asynchronous event notification.
 * 
 * Copyright (c) 2024 LimitlessOS Project
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

// Signal numbers (POSIX)
#define SIGHUP     1
#define SIGINT     2
#define SIGQUIT    3
#define SIGILL     4
#define SIGTRAP    5
#define SIGABRT    6
#define SIGBUS     7
#define SIGFPE     8
#define SIGKILL    9
#define SIGUSR1    10
#define SIGSEGV    11
#define SIGUSR2    12
#define SIGPIPE    13
#define SIGALRM    14
#define SIGTERM    15
#define SIGCHLD    17
#define SIGCONT    18
#define SIGSTOP    19
#define SIGTSTP    20
#define SIGTTIN    21
#define SIGTTOU    22
#define SIGWINCH   28
#define NSIG       32

typedef void (*signal_handler_t)(int);

#define SIG_DFL ((signal_handler_t)0)
#define SIG_IGN ((signal_handler_t)1)
#define SIG_ERR ((signal_handler_t)-1)

// Per-process signal information
typedef struct {
    signal_handler_t handlers[NSIG];
    uint32_t pending;     // Bitmap of pending signals
    uint32_t blocked;     // Bitmap of blocked signals
} signal_state_t;

// Global signal state (simplified - should be per-process)
static signal_state_t global_signal_state;

// Initialize signal system
void signal_init(void) {
    // Set all handlers to default
    for (int i = 0; i < NSIG; i++) {
        global_signal_state.handlers[i] = SIG_DFL;
    }
    global_signal_state.pending = 0;
    global_signal_state.blocked = 0;
}

// Default signal actions
static void signal_default_action(int sig) {
    switch (sig) {
        case SIGCHLD:
        case SIGCONT:
        case SIGWINCH:
            // Ignore by default
            break;
            
        case SIGSTOP:
        case SIGTSTP:
        case SIGTTIN:
        case SIGTTOU:
            // Stop process
            // TODO: Implement process stopping
            break;
            
        case SIGKILL:
        case SIGTERM:
        case SIGINT:
        case SIGQUIT:
        case SIGABRT:
        case SIGSEGV:
        case SIGILL:
        case SIGFPE:
        case SIGBUS:
            // Terminate process
            // TODO: Call process termination
            break;
            
        default:
            // Terminate
            break;
    }
}

// Send signal to process
int sys_kill(int pid, int sig) {
    if (sig < 1 || sig >= NSIG) {
        return -1;  // Invalid signal
    }
    
    // TODO: Find process by PID and set pending signal
    global_signal_state.pending |= (1U << sig);
    
    return 0;
}

// Set signal handler
signal_handler_t sys_signal(int sig, signal_handler_t handler) {
    if (sig < 1 || sig >= NSIG) {
        return SIG_ERR;
    }
    
    // SIGKILL and SIGSTOP cannot be caught
    if (sig == SIGKILL || sig == SIGSTOP) {
        return SIG_ERR;
    }
    
    signal_handler_t old_handler = global_signal_state.handlers[sig];
    global_signal_state.handlers[sig] = handler;
    
    return old_handler;
}

// Check and handle pending signals
void signal_handle_pending(void) {
    uint32_t pending = global_signal_state.pending & ~global_signal_state.blocked;
    
    if (pending == 0) {
        return;  // No pending signals
    }
    
    // Find first pending signal
    for (int sig = 1; sig < NSIG; sig++) {
        if (pending & (1U << sig)) {
            // Clear pending bit
            global_signal_state.pending &= ~(1U << sig);
            
            // Get handler
            signal_handler_t handler = global_signal_state.handlers[sig];
            
            if (handler == SIG_IGN) {
                // Ignore signal
                continue;
            } else if (handler == SIG_DFL) {
                // Default action
                signal_default_action(sig);
            } else {
                // Call user handler
                handler(sig);
            }
            
            break;  // Handle one signal at a time
        }
    }
}

// Block/unblock signals
int sys_sigprocmask(int how, const uint32_t *set, uint32_t *oldset) {
    if (oldset) {
        *oldset = global_signal_state.blocked;
    }
    
    if (set) {
        switch (how) {
            case 0:  // SIG_BLOCK
                global_signal_state.blocked |= *set;
                break;
            case 1:  // SIG_UNBLOCK
                global_signal_state.blocked &= ~(*set);
                break;
            case 2:  // SIG_SETMASK
                global_signal_state.blocked = *set;
                break;
            default:
                return -1;
        }
        
        // Can't block SIGKILL or SIGSTOP
        global_signal_state.blocked &= ~((1U << SIGKILL) | (1U << SIGSTOP));
    }
    
    return 0;
}

// Get pending signals
int sys_sigpending(uint32_t *set) {
    if (set) {
        *set = global_signal_state.pending;
        return 0;
    }
    return -1;
}

// Alarm timer (simplified)
static uint32_t alarm_time = 0;

unsigned int sys_alarm(unsigned int seconds) {
    unsigned int old_alarm = alarm_time;
    alarm_time = seconds;
    
    // TODO: Set up timer to send SIGALRM after 'seconds'
    
    return old_alarm;
}

// Pause until signal
int sys_pause(void) {
    // Wait for any signal
    while (global_signal_state.pending == 0) {
        // TODO: Put process to sleep
        asm volatile("hlt");
    }
    return -1;  // Always returns -1 with errno = EINTR
}
