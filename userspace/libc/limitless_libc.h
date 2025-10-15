/**
 * LimitlessOS Complete C Library Implementation
 * 
 * Full-featured C library providing all standard functions on par with
 * glibc, musl, and other production C libraries. Includes POSIX compliance,
 * thread safety, and advanced features for modern development.
 * 
 * Copyright (c) 2024 LimitlessOS Project
 */

#ifndef __LIMITLESS_LIBC_H__
#define __LIMITLESS_LIBC_H__

/* Standard C Library Headers */
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <limits.h>
#include <float.h>
#include <errno.h>

/* POSIX and System Headers */
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <pthread.h>
#include <semaphore.h>
#include <dirent.h>
#include <time.h>

/* Network Headers */
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

/* Advanced Features */
#include <regex.h>
#include <locale.h>
#include <iconv.h>
#include <dlfcn.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ===== MEMORY MANAGEMENT ===== */

/**
 * Enhanced memory allocator with debugging and statistics
 */
void *limitless_malloc(size_t size);
void *limitless_calloc(size_t nmemb, size_t size);
void *limitless_realloc(void *ptr, size_t size);
void limitless_free(void *ptr);

/* Aligned memory allocation */
void *limitless_aligned_alloc(size_t alignment, size_t size);
int limitless_posix_memalign(void **memptr, size_t alignment, size_t size);

/* Memory debugging */
void *limitless_malloc_debug(size_t size, const char *file, int line);
void limitless_free_debug(void *ptr, const char *file, int line);
void limitless_memory_stats(void);
int limitless_memory_check(void);

#define malloc(size) limitless_malloc_debug(size, __FILE__, __LINE__)
#define free(ptr) limitless_free_debug(ptr, __FILE__, __LINE__)

/* ===== STRING FUNCTIONS ===== */

/* Standard string functions */
size_t limitless_strlen(const char *s);
char *limitless_strcpy(char *dest, const char *src);
char *limitless_strncpy(char *dest, const char *src, size_t n);
char *limitless_strcat(char *dest, const char *src);
char *limitless_strncat(char *dest, const char *src, size_t n);
int limitless_strcmp(const char *s1, const char *s2);
int limitless_strncmp(const char *s1, const char *s2, size_t n);
int limitless_strcasecmp(const char *s1, const char *s2);
char *limitless_strchr(const char *s, int c);
char *limitless_strrchr(const char *s, int c);
char *limitless_strstr(const char *haystack, const char *needle);
char *limitless_strtok(char *str, const char *delim);
char *limitless_strtok_r(char *str, const char *delim, char **saveptr);

/* Memory functions */
void *limitless_memcpy(void *dest, const void *src, size_t n);
void *limitless_memmove(void *dest, const void *src, size_t n);
void *limitless_memset(void *s, int c, size_t n);
int limitless_memcmp(const void *s1, const void *s2, size_t n);
void *limitless_memchr(const void *s, int c, size_t n);

/* String conversion */
long limitless_strtol(const char *nptr, char **endptr, int base);
unsigned long limitless_strtoul(const char *nptr, char **endptr, int base);
long long limitless_strtoll(const char *nptr, char **endptr, int base);
unsigned long long limitless_strtoull(const char *nptr, char **endptr, int base);
double limitless_strtod(const char *nptr, char **endptr);
float limitless_strtof(const char *nptr, char **endptr);
long double limitless_strtold(const char *nptr, char **endptr);

/* String formatting */
int limitless_sprintf(char *str, const char *format, ...);
int limitless_snprintf(char *str, size_t size, const char *format, ...);
int limitless_vsprintf(char *str, const char *format, va_list ap);
int limitless_vsnprintf(char *str, size_t size, const char *format, va_list ap);
int limitless_asprintf(char **strp, const char *format, ...);

/* ===== INPUT/OUTPUT ===== */

/* File operations */
typedef struct limitless_file {
    int fd;                         /* File descriptor */
    char *buffer;                   /* I/O buffer */
    size_t buffer_size;             /* Buffer size */
    size_t buffer_pos;              /* Current buffer position */
    size_t buffer_end;              /* End of valid data in buffer */
    int flags;                      /* File flags */
    int error;                      /* Error state */
    bool eof;                       /* End of file reached */
    pthread_mutex_t lock;           /* Thread safety */
} LIMITLESS_FILE;

#define FILE LIMITLESS_FILE

/* Standard streams */
extern FILE *limitless_stdin;
extern FILE *limitless_stdout;
extern FILE *limitless_stderr;

#define stdin limitless_stdin
#define stdout limitless_stdout  
#define stderr limitless_stderr

/* File operations */
FILE *limitless_fopen(const char *pathname, const char *mode);
FILE *limitless_fdopen(int fd, const char *mode);
FILE *limitless_freopen(const char *pathname, const char *mode, FILE *stream);
int limitless_fclose(FILE *stream);
int limitless_fflush(FILE *stream);

/* Character I/O */
int limitless_fgetc(FILE *stream);
int limitless_getc(FILE *stream);
int limitless_getchar(void);
int limitless_ungetc(int c, FILE *stream);
int limitless_fputc(int c, FILE *stream);
int limitless_putc(int c, FILE *stream);
int limitless_putchar(int c);

/* String I/O */
char *limitless_fgets(char *s, int size, FILE *stream);
char *limitless_gets(char *s);
int limitless_fputs(const char *s, FILE *stream);
int limitless_puts(const char *s);

/* Formatted I/O */
int limitless_printf(const char *format, ...);
int limitless_fprintf(FILE *stream, const char *format, ...);
int limitless_vprintf(const char *format, va_list ap);
int limitless_vfprintf(FILE *stream, const char *format, va_list ap);

int limitless_scanf(const char *format, ...);
int limitless_fscanf(FILE *stream, const char *format, ...);
int limitless_sscanf(const char *str, const char *format, ...);
int limitless_vscanf(const char *format, va_list ap);
int limitless_vfscanf(FILE *stream, const char *format, va_list ap);
int limitless_vsscanf(const char *str, const char *format, va_list ap);

/* Binary I/O */
size_t limitless_fread(void *ptr, size_t size, size_t nmemb, FILE *stream);
size_t limitless_fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream);

/* File positioning */
int limitless_fseek(FILE *stream, long offset, int whence);
long limitless_ftell(FILE *stream);
void limitless_rewind(FILE *stream);
int limitless_fgetpos(FILE *stream, fpos_t *pos);
int limitless_fsetpos(FILE *stream, const fpos_t *pos);

/* File status */
int limitless_feof(FILE *stream);
int limitless_ferror(FILE *stream);
void limitless_clearerr(FILE *stream);

/* ===== MATHEMATICAL FUNCTIONS ===== */

/* Basic math */
double limitless_fabs(double x);
float limitless_fabsf(float x);
long double limitless_fabsl(long double x);

double limitless_ceil(double x);
float limitless_ceilf(float x);
long double limitless_ceill(long double x);

double limitless_floor(double x);
float limitless_floorf(float x);
long double limitless_floorl(long double x);

double limitless_round(double x);
float limitless_roundf(float x);
long double limitless_roundl(long double x);

double limitless_trunc(double x);
float limitless_truncf(float x);
long double limitless_truncl(long double x);

/* Power and exponential functions */
double limitless_pow(double x, double y);
float limitless_powf(float x, float y);
long double limitless_powl(long double x, long double y);

double limitless_sqrt(double x);
float limitless_sqrtf(float x);
long double limitless_sqrtl(long double x);

double limitless_cbrt(double x);
float limitless_cbrtf(float x);
long double limitless_cbrtl(long double x);

double limitless_exp(double x);
float limitless_expf(float x);
long double limitless_expl(long double x);

double limitless_exp2(double x);
float limitless_exp2f(float x);
long double limitless_exp2l(long double x);

double limitless_log(double x);
float limitless_logf(float x);
long double limitless_logl(long double x);

double limitless_log10(double x);
float limitless_log10f(float x);
long double limitless_log10l(long double x);

double limitless_log2(double x);
float limitless_log2f(float x);
long double limitless_log2l(long double x);

/* Trigonometric functions */
double limitless_sin(double x);
float limitless_sinf(float x);
long double limitless_sinl(long double x);

double limitless_cos(double x);
float limitless_cosf(float x);
long double limitless_cosl(long double x);

double limitless_tan(double x);
float limitless_tanf(float x);
long double limitless_tanl(long double x);

double limitless_asin(double x);
float limitless_asinf(float x);
long double limitless_asinl(long double x);

double limitless_acos(double x);
float limitless_acosf(float x);
long double limitless_acosl(long double x);

double limitless_atan(double x);
float limitless_atanf(float x);
long double limitless_atanl(long double x);

double limitless_atan2(double y, double x);
float limitless_atan2f(float y, float x);
long double limitless_atan2l(long double y, long double x);

/* Hyperbolic functions */
double limitless_sinh(double x);
float limitless_sinhf(float x);
long double limitless_sinhl(long double x);

double limitless_cosh(double x);
float limitless_coshf(float x);
long double limitless_coshl(long double x);

double limitless_tanh(double x);
float limitless_tanhf(float x);
long double limitless_tanhl(long double x);

/* ===== SYSTEM CALLS AND POSIX ===== */

/* Process control */
pid_t limitless_fork(void);
int limitless_execve(const char *pathname, char *const argv[], char *const envp[]);
int limitless_execv(const char *pathname, char *const argv[]);
int limitless_execvp(const char *file, char *const argv[]);
int limitless_system(const char *command);
void limitless_exit(int status);
void limitless_abort(void);
int limitless_atexit(void (*function)(void));

pid_t limitless_wait(int *wstatus);
pid_t limitless_waitpid(pid_t pid, int *wstatus, int options);

/* File system operations */
int limitless_open(const char *pathname, int flags, ...);
int limitless_close(int fd);
ssize_t limitless_read(int fd, void *buf, size_t count);
ssize_t limitless_write(int fd, const void *buf, size_t count);
off_t limitless_lseek(int fd, off_t offset, int whence);
int limitless_fsync(int fd);
int limitless_fdatasync(int fd);

int limitless_stat(const char *pathname, struct stat *statbuf);
int limitless_fstat(int fd, struct stat *statbuf);
int limitless_lstat(const char *pathname, struct stat *statbuf);

int limitless_mkdir(const char *pathname, mode_t mode);
int limitless_rmdir(const char *pathname);
int limitless_unlink(const char *pathname);
int limitless_link(const char *oldpath, const char *newpath);
int limitless_symlink(const char *target, const char *linkpath);
ssize_t limitless_readlink(const char *pathname, char *buf, size_t bufsiz);

int limitless_chmod(const char *pathname, mode_t mode);
int limitless_fchmod(int fd, mode_t mode);
int limitless_chown(const char *pathname, uid_t owner, gid_t group);
int limitless_fchown(int fd, uid_t owner, gid_t group);

int limitless_rename(const char *oldpath, const char *newpath);
int limitless_access(const char *pathname, int mode);
char *limitless_getcwd(char *buf, size_t size);
int limitless_chdir(const char *path);

/* Directory operations */
DIR *limitless_opendir(const char *name);
struct dirent *limitless_readdir(DIR *dirp);
int limitless_readdir_r(DIR *dirp, struct dirent *entry, struct dirent **result);
int limitless_closedir(DIR *dirp);
void limitless_rewinddir(DIR *dirp);
long limitless_telldir(DIR *dirp);
void limitless_seekdir(DIR *dirp, long loc);

/* Memory management system calls */
void *limitless_mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
int limitless_munmap(void *addr, size_t length);
int limitless_mprotect(void *addr, size_t len, int prot);
int limitless_mlock(const void *addr, size_t len);
int limitless_munlock(const void *addr, size_t len);

/* Signal handling */
typedef void (*limitless_sighandler_t)(int);
limitless_sighandler_t limitless_signal(int signum, limitless_sighandler_t handler);
int limitless_sigaction(int signum, const struct sigaction *act, struct sigaction *oldact);
int limitless_kill(pid_t pid, int sig);
int limitless_raise(int sig);
int limitless_pause(void);
unsigned int limitless_alarm(unsigned int seconds);

/* Time functions */
time_t limitless_time(time_t *tloc);
int limitless_gettimeofday(struct timeval *tv, struct timezone *tz);
int limitless_settimeofday(const struct timeval *tv, const struct timezone *tz);
struct tm *limitless_localtime(const time_t *timep);
struct tm *limitless_gmtime(const time_t *timep);
time_t limitless_mktime(struct tm *tm);
char *limitless_ctime(const time_t *timep);
char *limitless_asctime(const struct tm *tm);
size_t limitless_strftime(char *s, size_t max, const char *format, const struct tm *tm);

int limitless_nanosleep(const struct timespec *req, struct timespec *rem);
int limitless_usleep(useconds_t usec);
int limitless_sleep(unsigned int seconds);

/* ===== THREADING (POSIX Threads) ===== */

/* Thread management */
int limitless_pthread_create(pthread_t *thread, const pthread_attr_t *attr,
                           void *(*start_routine)(void *), void *arg);
void limitless_pthread_exit(void *retval);
int limitless_pthread_join(pthread_t thread, void **retval);
int limitless_pthread_detach(pthread_t thread);
pthread_t limitless_pthread_self(void);
int limitless_pthread_equal(pthread_t t1, pthread_t t2);

/* Thread attributes */
int limitless_pthread_attr_init(pthread_attr_t *attr);
int limitless_pthread_attr_destroy(pthread_attr_t *attr);
int limitless_pthread_attr_setdetachstate(pthread_attr_t *attr, int detachstate);
int limitless_pthread_attr_getdetachstate(const pthread_attr_t *attr, int *detachstate);
int limitless_pthread_attr_setstacksize(pthread_attr_t *attr, size_t stacksize);
int limitless_pthread_attr_getstacksize(const pthread_attr_t *attr, size_t *stacksize);

/* Mutexes */
int limitless_pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr);
int limitless_pthread_mutex_destroy(pthread_mutex_t *mutex);
int limitless_pthread_mutex_lock(pthread_mutex_t *mutex);
int limitless_pthread_mutex_trylock(pthread_mutex_t *mutex);
int limitless_pthread_mutex_unlock(pthread_mutex_t *mutex);

/* Condition variables */
int limitless_pthread_cond_init(pthread_cond_t *cond, const pthread_condattr_t *attr);
int limitless_pthread_cond_destroy(pthread_cond_t *cond);
int limitless_pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex);
int limitless_pthread_cond_timedwait(pthread_cond_t *cond, pthread_mutex_t *mutex,
                                   const struct timespec *abstime);
int limitless_pthread_cond_signal(pthread_cond_t *cond);
int limitless_pthread_cond_broadcast(pthread_cond_t *cond);

/* Read-write locks */
int limitless_pthread_rwlock_init(pthread_rwlock_t *rwlock, const pthread_rwlockattr_t *attr);
int limitless_pthread_rwlock_destroy(pthread_rwlock_t *rwlock);
int limitless_pthread_rwlock_rdlock(pthread_rwlock_t *rwlock);
int limitless_pthread_rwlock_tryrdlock(pthread_rwlock_t *rwlock);
int limitless_pthread_rwlock_wrlock(pthread_rwlock_t *rwlock);
int limitless_pthread_rwlock_trywrlock(pthread_rwlock_t *rwlock);
int limitless_pthread_rwlock_unlock(pthread_rwlock_t *rwlock);

/* Semaphores */
int limitless_sem_init(sem_t *sem, int pshared, unsigned int value);
int limitless_sem_destroy(sem_t *sem);
int limitless_sem_wait(sem_t *sem);
int limitless_sem_trywait(sem_t *sem);
int limitless_sem_timedwait(sem_t *sem, const struct timespec *abs_timeout);
int limitless_sem_post(sem_t *sem);
int limitless_sem_getvalue(sem_t *sem, int *sval);

/* ===== NETWORKING ===== */

/* Socket operations */
int limitless_socket(int domain, int type, int protocol);
int limitless_bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
int limitless_listen(int sockfd, int backlog);
int limitless_accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
int limitless_connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
ssize_t limitless_send(int sockfd, const void *buf, size_t len, int flags);
ssize_t limitless_recv(int sockfd, void *buf, size_t len, int flags);
ssize_t limitless_sendto(int sockfd, const void *buf, size_t len, int flags,
                        const struct sockaddr *dest_addr, socklen_t addrlen);
ssize_t limitless_recvfrom(int sockfd, void *buf, size_t len, int flags,
                          struct sockaddr *src_addr, socklen_t *addrlen);
int limitless_shutdown(int sockfd, int how);
int limitless_setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen);
int limitless_getsockopt(int sockfd, int level, int optname, void *optval, socklen_t *optlen);

/* Name resolution */
struct hostent *limitless_gethostbyname(const char *name);
struct hostent *limitless_gethostbyaddr(const void *addr, socklen_t len, int type);
int limitless_getaddrinfo(const char *node, const char *service,
                         const struct addrinfo *hints, struct addrinfo **res);
void limitless_freeaddrinfo(struct addrinfo *res);
const char *limitless_gai_strerror(int errcode);

/* Address conversion */
const char *limitless_inet_ntop(int af, const void *src, char *dst, socklen_t size);
int limitless_inet_pton(int af, const char *src, void *dst);
char *limitless_inet_ntoa(struct in_addr in);
in_addr_t limitless_inet_addr(const char *cp);

/* ===== REGULAR EXPRESSIONS ===== */

int limitless_regcomp(regex_t *preg, const char *regex, int cflags);
int limitless_regexec(const regex_t *preg, const char *string, size_t nmatch,
                     regmatch_t pmatch[], int eflags);
size_t limitless_regerror(int errcode, const regex_t *preg, char *errbuf, size_t errbuf_size);
void limitless_regfree(regex_t *preg);

/* ===== DYNAMIC LOADING ===== */

void *limitless_dlopen(const char *filename, int flags);
char *limitless_dlerror(void);
void *limitless_dlsym(void *handle, const char *symbol);
int limitless_dlclose(void *handle);

/* ===== LOCALE AND INTERNATIONALIZATION ===== */

char *limitless_setlocale(int category, const char *locale);
struct lconv *limitless_localeconv(void);

/* Character conversion */
iconv_t limitless_iconv_open(const char *tocode, const char *fromcode);
size_t limitless_iconv(iconv_t cd, char **inbuf, size_t *inbytesleft,
                      char **outbuf, size_t *outbytesleft);
int limitless_iconv_close(iconv_t cd);

/* Wide character support */
size_t limitless_mblen(const char *s, size_t n);
int limitless_mbtowc(wchar_t *pwc, const char *s, size_t n);
int limitless_wctomb(char *s, wchar_t wc);
size_t limitless_mbstowcs(wchar_t *dest, const char *src, size_t n);
size_t limitless_wcstombs(char *dest, const wchar_t *src, size_t n);

/* ===== ERROR HANDLING ===== */

extern int limitless_errno;
#define errno limitless_errno

char *limitless_strerror(int errnum);
int limitless_strerror_r(int errnum, char *buf, size_t buflen);
void limitless_perror(const char *s);

/* ===== RANDOM NUMBERS ===== */

int limitless_rand(void);
void limitless_srand(unsigned int seed);
long limitless_random(void);
void limitless_srandom(unsigned int seed);
char *limitless_initstate(unsigned int seed, char *state, size_t n);
char *limitless_setstate(char *state);

/* ===== ENVIRONMENT ===== */

char *limitless_getenv(const char *name);
int limitless_setenv(const char *name, const char *value, int overwrite);
int limitless_unsetenv(const char *name);
int limitless_putenv(char *string);
extern char **limitless_environ;
#define environ limitless_environ

/* ===== MISCELLANEOUS ===== */

int limitless_abs(int j);
long limitless_labs(long j);
long long limitless_llabs(long long j);

div_t limitless_div(int numer, int denom);
ldiv_t limitless_ldiv(long numer, long denom);
lldiv_t limitless_lldiv(long long numer, long long denom);

int limitless_atexit(void (*function)(void));
int limitless_on_exit(void (*function)(int, void *), void *arg);

void limitless_qsort(void *base, size_t nmemb, size_t size,
                    int (*compar)(const void *, const void *));
void *limitless_bsearch(const void *key, const void *base, size_t nmemb,
                       size_t size, int (*compar)(const void *, const void *));

/* ===== LIBC STATUS AND DEBUGGING ===== */

typedef struct libc_stats {
    uint64_t malloc_calls;
    uint64_t free_calls;
    uint64_t malloc_bytes;
    uint64_t free_bytes;
    uint64_t memory_leaks;
    
    uint64_t file_opens;
    uint64_t file_closes;
    uint64_t bytes_read;
    uint64_t bytes_written;
    
    uint64_t thread_creates;
    uint64_t mutex_locks;
    uint64_t mutex_unlocks;
    
    uint64_t network_connects;
    uint64_t network_bytes_sent;
    uint64_t network_bytes_received;
} libc_stats_t;

void limitless_libc_get_stats(libc_stats_t *stats);
void limitless_libc_print_stats(void);
int limitless_libc_check_integrity(void);
void limitless_libc_set_debug_level(int level);

#ifdef __cplusplus
}
#endif

#endif /* __LIMITLESS_LIBC_H__ */