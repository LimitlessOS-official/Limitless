#ifndef _UNISTD_H
#define _UNISTD_H

#include <stddef.h>

// Standard types
typedef int pid_t;
typedef long ssize_t;
typedef long off_t;

// Process control
pid_t getpid(void);
pid_t getppid(void);
pid_t fork(void);
int execve(const char *path, char *const argv[], char *const envp[]);
void exit(int status) __attribute__((noreturn));
pid_t waitpid(pid_t pid, int *status, int options);
pid_t wait(int *status);

// Sleep functions
unsigned int sleep(unsigned int seconds);
int usleep(unsigned int usec);

// I/O
ssize_t read(int fd, void *buf, size_t count);
ssize_t write(int fd, const void *buf, size_t count);
int close(int fd);
off_t lseek(int fd, off_t offset, int whence);

// File system
int chdir(const char *path);
char *getcwd(char *buf, size_t size);

// Pipe
int pipe(int pipefd[2]);

// Standard file descriptors
#define STDIN_FILENO  0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

// lseek whence values
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

#endif // _UNISTD_H
