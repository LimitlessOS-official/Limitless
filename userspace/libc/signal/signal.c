#include <syscall.h>

// Signal management
typedef void (*sighandler_t)(int);

#define SIG_DFL ((sighandler_t)0)
#define SIG_IGN ((sighandler_t)1)
#define SIG_ERR ((sighandler_t)-1)

sighandler_t signal(int signum, sighandler_t handler) {
    return (sighandler_t)syscall2(SYS_SIGNAL, signum, (int)handler);
}

int kill(int pid, int sig) {
    return syscall2(SYS_KILL, pid, sig);
}

unsigned int alarm(unsigned int seconds) {
    return syscall1(SYS_ALARM, seconds);
}

int pause(void) {
    return syscall0(SYS_PAUSE);
}

int sigprocmask(int how, const unsigned int *set, unsigned int *oldset) {
    return syscall3(SYS_SIGPROCMASK, how, (int)set, (int)oldset);
}

int sigpending(unsigned int *set) {
    return syscall1(SYS_SIGPENDING, (int)set);
}
