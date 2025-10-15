#include <sys/ipc.h>
#include <syscall.h>

// Syscall function declarations
extern int syscall1(int num, int arg1);
extern int syscall2(int num, int arg1, int arg2);
extern int syscall3(int num, int arg1, int arg2, int arg3);
extern int syscall4(int num, int arg1, int arg2, int arg3, int arg4);
extern int syscall5(int num, int arg1, int arg2, int arg3, int arg4, int arg5);

// Pipe: Create a pipe
int pipe(int pipefd[2]) {
    return syscall1(SYS_PIPE, (int)pipefd);
}

// Shared memory operations
int shmget(int key, size_t size, int shmflg) {
    return syscall3(SYS_SHMGET, key, (int)size, shmflg);
}

void *shmat(int shmid, const void *shmaddr, int shmflg) {
    return (void *)syscall3(SYS_SHMAT, shmid, (int)shmaddr, shmflg);
}

int shmdt(const void *shmaddr) {
    return syscall1(SYS_SHMDT, (int)shmaddr);
}

// Semaphore operations
int semget(int key, int nsems, int semflg) {
    return syscall3(SYS_SEMGET, key, nsems, semflg);
}

int semop(int semid, struct sembuf *sops, size_t nsops) {
    return syscall3(SYS_SEMOP, semid, (int)sops, (int)nsops);
}

// Message queue operations  
int msgget(int key, int msgflg) {
    return syscall2(SYS_MSGGET, key, msgflg);
}

int msgsnd(int msqid, const void *msgp, size_t msgsz, int msgflg) {
    return syscall4(SYS_MSGSND, msqid, (int)msgp, (int)msgsz, msgflg);
}

ssize_t msgrcv(int msqid, void *msgp, size_t msgsz, long msgtyp, int msgflg) {
    return syscall5(SYS_MSGRCV, msqid, (int)msgp, (int)msgsz, (int)msgtyp, msgflg);
}
