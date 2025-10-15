#ifndef _SYS_IPC_H
#define _SYS_IPC_H

#include <stddef.h>

// IPC key type
typedef int key_t;
typedef long ssize_t;

// IPC flags
#define IPC_PRIVATE 0
#define IPC_CREAT   0x0200
#define IPC_EXCL    0x0400
#define IPC_NOWAIT  0x0800

// Shared memory flags
#define SHM_RDONLY  0x1000
#define SHM_RND     0x2000
#define SHM_REMAP   0x4000

// Shared memory operations
int shmget(key_t key, size_t size, int shmflg);
void *shmat(int shmid, const void *shmaddr, int shmflg);
int shmdt(const void *shmaddr);

// Semaphore operations
#define SEM_UNDO 0x1000

struct sembuf {
    unsigned short sem_num;
    short sem_op;
    short sem_flg;
};

int semget(key_t key, int nsems, int semflg);
int semop(int semid, struct sembuf *sops, size_t nsops);

// Message queue operations
#define MSG_MAX_SIZE 8192

struct msgbuf {
    long mtype;
    char mtext[MSG_MAX_SIZE];
};

int msgget(key_t key, int msgflg);
int msgsnd(int msqid, const void *msgp, size_t msgsz, int msgflg);
ssize_t msgrcv(int msqid, void *msgp, size_t msgsz, long msgtyp, int msgflg);

// Pipe
int pipe(int pipefd[2]);

#endif // _SYS_IPC_H
