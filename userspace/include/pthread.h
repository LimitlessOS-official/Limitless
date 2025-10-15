/*
 * pthread.h - LimitlessOS POSIX Threads Header
 * 
 * Complete pthread implementation with mutexes, condition variables,
 * thread-local storage, and cancellation support.
 */

#ifndef LIMITLESSOS_PTHREAD_H
#define LIMITLESSOS_PTHREAD_H

#include <kernel/types.h>
#include <kernel/time.h>

/* Thread types */
typedef unsigned long pthread_t;
typedef struct pthread_attr pthread_attr_t;
typedef struct pthread_mutex pthread_mutex_t;
typedef struct pthread_mutexattr pthread_mutexattr_t;
typedef struct pthread_cond pthread_cond_t;
typedef struct pthread_condattr pthread_condattr_t;
typedef struct pthread_rwlock pthread_rwlock_t;
typedef struct pthread_rwlockattr pthread_rwlockattr_t;
typedef struct pthread_spinlock pthread_spinlock_t;
typedef struct pthread_barrier pthread_barrier_t;
typedef struct pthread_barrierattr pthread_barrierattr_t;
typedef unsigned int pthread_key_t;
typedef int pthread_once_t;

/* Constants */
#define PTHREAD_KEYS_MAX            1024
#define PTHREAD_STACK_MIN          16384
#define PTHREAD_THREADS_MAX         1024

/* Thread creation states */
#define PTHREAD_CREATE_JOINABLE     0
#define PTHREAD_CREATE_DETACHED     1

/* Mutex types */
#define PTHREAD_MUTEX_NORMAL        0
#define PTHREAD_MUTEX_RECURSIVE     1
#define PTHREAD_MUTEX_ERRORCHECK    2
#define PTHREAD_MUTEX_DEFAULT       PTHREAD_MUTEX_NORMAL

/* Mutex protocols */
#define PTHREAD_PRIO_NONE           0
#define PTHREAD_PRIO_INHERIT        1
#define PTHREAD_PRIO_PROTECT        2

/* Process shared */
#define PTHREAD_PROCESS_PRIVATE     0
#define PTHREAD_PROCESS_SHARED      1

/* Mutex robustness */
#define PTHREAD_MUTEX_STALLED       0
#define PTHREAD_MUTEX_ROBUST        1

/* Cancellation */
#define PTHREAD_CANCEL_ENABLE       0
#define PTHREAD_CANCEL_DISABLE      1
#define PTHREAD_CANCEL_DEFERRED     0
#define PTHREAD_CANCEL_ASYNCHRONOUS 1

#define PTHREAD_CANCELED            ((void*)-1)

/* Once control */
#define PTHREAD_ONCE_INIT           0

/* Thread attributes */
struct pthread_attr {
    int detachstate;
    int schedpolicy;
    struct sched_param schedparam;
    int inheritsched;
    int scope;
    size_t guardsize;
    void* stackaddr;
    size_t stacksize;
};

/* Mutex */
struct pthread_mutex {
    volatile int futex_word;
    int type;
    int protocol;
    int pshared;
    int robust;
    pthread_t owner;
    unsigned int lock_count;
    int prioceiling;
};

/* Mutex attributes */
struct pthread_mutexattr {
    int type;
    int protocol;
    int pshared;
    int robust;
    int prioceiling;
};

/* Condition variable */
struct pthread_cond {
    volatile int futex_word;
    int pshared;
    clockid_t clock;
    pthread_mutex_t* mutex;
    volatile int waiters;
};

/* Condition variable attributes */
struct pthread_condattr {
    int pshared;
    clockid_t clock;
};

/* Reader-writer lock */
struct pthread_rwlock {
    volatile int futex_word;
    int pshared;
    int kind;
    unsigned int nr_readers;
    unsigned int nr_writers_queued;
    unsigned int nr_readers_queued;
    pthread_t writer;
};

/* Reader-writer lock attributes */
struct pthread_rwlockattr {
    int pshared;
    int kind;
};

/* Spinlock */
struct pthread_spinlock {
    volatile int lock;
    int pshared;
};

/* Barrier */
struct pthread_barrier {
    volatile int futex_word;
    unsigned int count;
    unsigned int total;
    unsigned int generation;
    int pshared;
};

/* Barrier attributes */
struct pthread_barrierattr {
    int pshared;
};

/* Thread management */
int pthread_create(pthread_t* thread, const pthread_attr_t* attr,
                   void* (*start_routine)(void*), void* arg);
int pthread_join(pthread_t thread, void** retval);
int pthread_detach(pthread_t thread);
void pthread_exit(void* retval) __attribute__((noreturn));
pthread_t pthread_self(void);
int pthread_equal(pthread_t t1, pthread_t t2);

/* Thread attributes */
int pthread_attr_init(pthread_attr_t* attr);
int pthread_attr_destroy(pthread_attr_t* attr);
int pthread_attr_getdetachstate(const pthread_attr_t* attr, int* detachstate);
int pthread_attr_setdetachstate(pthread_attr_t* attr, int detachstate);
int pthread_attr_getguardsize(const pthread_attr_t* attr, size_t* guardsize);
int pthread_attr_setguardsize(pthread_attr_t* attr, size_t guardsize);
int pthread_attr_getschedparam(const pthread_attr_t* attr, struct sched_param* param);
int pthread_attr_setschedparam(pthread_attr_t* attr, const struct sched_param* param);
int pthread_attr_getschedpolicy(const pthread_attr_t* attr, int* policy);
int pthread_attr_setschedpolicy(pthread_attr_t* attr, int policy);
int pthread_attr_getinheritsched(const pthread_attr_t* attr, int* inheritsched);
int pthread_attr_setinheritsched(pthread_attr_t* attr, int inheritsched);
int pthread_attr_getscope(const pthread_attr_t* attr, int* scope);
int pthread_attr_setscope(pthread_attr_t* attr, int scope);
int pthread_attr_getstack(const pthread_attr_t* attr, void** stackaddr, size_t* stacksize);
int pthread_attr_setstack(pthread_attr_t* attr, void* stackaddr, size_t stacksize);
int pthread_attr_getstackaddr(const pthread_attr_t* attr, void** stackaddr);
int pthread_attr_setstackaddr(pthread_attr_t* attr, void* stackaddr);
int pthread_attr_getstacksize(const pthread_attr_t* attr, size_t* stacksize);
int pthread_attr_setstacksize(pthread_attr_t* attr, size_t stacksize);

/* Cancellation */
int pthread_cancel(pthread_t thread);
int pthread_setcancelstate(int state, int* oldstate);
int pthread_setcanceltype(int type, int* oldtype);
void pthread_testcancel(void);

/* Cleanup handlers */
void pthread_cleanup_push(void (*routine)(void*), void* arg);
void pthread_cleanup_pop(int execute);

/* Mutex */
int pthread_mutex_init(pthread_mutex_t* mutex, const pthread_mutexattr_t* attr);
int pthread_mutex_destroy(pthread_mutex_t* mutex);
int pthread_mutex_lock(pthread_mutex_t* mutex);
int pthread_mutex_trylock(pthread_mutex_t* mutex);
int pthread_mutex_unlock(pthread_mutex_t* mutex);
int pthread_mutex_timedlock(pthread_mutex_t* mutex, const struct timespec* abs_timeout);

/* Mutex attributes */
int pthread_mutexattr_init(pthread_mutexattr_t* attr);
int pthread_mutexattr_destroy(pthread_mutexattr_t* attr);
int pthread_mutexattr_gettype(const pthread_mutexattr_t* attr, int* type);
int pthread_mutexattr_settype(pthread_mutexattr_t* attr, int type);
int pthread_mutexattr_getprotocol(const pthread_mutexattr_t* attr, int* protocol);
int pthread_mutexattr_setprotocol(pthread_mutexattr_t* attr, int protocol);
int pthread_mutexattr_getpshared(const pthread_mutexattr_t* attr, int* pshared);
int pthread_mutexattr_setpshared(pthread_mutexattr_t* attr, int pshared);
int pthread_mutexattr_getrobust(const pthread_mutexattr_t* attr, int* robust);
int pthread_mutexattr_setrobust(pthread_mutexattr_t* attr, int robust);

/* Condition variables */
int pthread_cond_init(pthread_cond_t* cond, const pthread_condattr_t* attr);
int pthread_cond_destroy(pthread_cond_t* cond);
int pthread_cond_wait(pthread_cond_t* cond, pthread_mutex_t* mutex);
int pthread_cond_timedwait(pthread_cond_t* cond, pthread_mutex_t* mutex,
                           const struct timespec* abs_timeout);
int pthread_cond_signal(pthread_cond_t* cond);
int pthread_cond_broadcast(pthread_cond_t* cond);

/* Condition variable attributes */
int pthread_condattr_init(pthread_condattr_t* attr);
int pthread_condattr_destroy(pthread_condattr_t* attr);
int pthread_condattr_getpshared(const pthread_condattr_t* attr, int* pshared);
int pthread_condattr_setpshared(pthread_condattr_t* attr, int pshared);
int pthread_condattr_getclock(const pthread_condattr_t* attr, clockid_t* clock_id);
int pthread_condattr_setclock(pthread_condattr_t* attr, clockid_t clock_id);

/* Reader-writer locks */
int pthread_rwlock_init(pthread_rwlock_t* rwlock, const pthread_rwlockattr_t* attr);
int pthread_rwlock_destroy(pthread_rwlock_t* rwlock);
int pthread_rwlock_rdlock(pthread_rwlock_t* rwlock);
int pthread_rwlock_tryrdlock(pthread_rwlock_t* rwlock);
int pthread_rwlock_timedrdlock(pthread_rwlock_t* rwlock, const struct timespec* abs_timeout);
int pthread_rwlock_wrlock(pthread_rwlock_t* rwlock);
int pthread_rwlock_trywrlock(pthread_rwlock_t* rwlock);
int pthread_rwlock_timedwrlock(pthread_rwlock_t* rwlock, const struct timespec* abs_timeout);
int pthread_rwlock_unlock(pthread_rwlock_t* rwlock);

/* Reader-writer lock attributes */
int pthread_rwlockattr_init(pthread_rwlockattr_t* attr);
int pthread_rwlockattr_destroy(pthread_rwlockattr_t* attr);
int pthread_rwlockattr_getpshared(const pthread_rwlockattr_t* attr, int* pshared);
int pthread_rwlockattr_setpshared(pthread_rwlockattr_t* attr, int pshared);

/* Spinlocks */
int pthread_spin_init(pthread_spinlock_t* lock, int pshared);
int pthread_spin_destroy(pthread_spinlock_t* lock);
int pthread_spin_lock(pthread_spinlock_t* lock);
int pthread_spin_trylock(pthread_spinlock_t* lock);
int pthread_spin_unlock(pthread_spinlock_t* lock);

/* Barriers */
int pthread_barrier_init(pthread_barrier_t* barrier, const pthread_barrierattr_t* attr,
                         unsigned count);
int pthread_barrier_destroy(pthread_barrier_t* barrier);
int pthread_barrier_wait(pthread_barrier_t* barrier);

/* Barrier attributes */
int pthread_barrierattr_init(pthread_barrierattr_t* attr);
int pthread_barrierattr_destroy(pthread_barrierattr_t* attr);
int pthread_barrierattr_getpshared(const pthread_barrierattr_t* attr, int* pshared);
int pthread_barrierattr_setpshared(pthread_barrierattr_t* attr, int pshared);

/* Thread-specific data */
int pthread_key_create(pthread_key_t* key, void (*destructor)(void*));
int pthread_key_delete(pthread_key_t key);
void* pthread_getspecific(pthread_key_t key);
int pthread_setspecific(pthread_key_t key, const void* value);

/* Once initialization */
int pthread_once(pthread_once_t* once_control, void (*init_routine)(void));

/* Scheduling */
int pthread_setschedparam(pthread_t thread, int policy, const struct sched_param* param);
int pthread_getschedparam(pthread_t thread, int* policy, struct sched_param* param);
int pthread_setschedprio(pthread_t thread, int prio);

/* Concurrency */
int pthread_getconcurrency(void);
int pthread_setconcurrency(int level);

/* CPU sets */
#define CPU_SETSIZE 1024
#define CPU_SET(cpu, cpusetp)   ((cpusetp)->__bits[(cpu)/64] |= (1UL << ((cpu) % 64)))
#define CPU_CLR(cpu, cpusetp)   ((cpusetp)->__bits[(cpu)/64] &= ~(1UL << ((cpu) % 64)))
#define CPU_ISSET(cpu, cpusetp) (((cpusetp)->__bits[(cpu)/64] & (1UL << ((cpu) % 64))) != 0)
#define CPU_ZERO(cpusetp)       memset((cpusetp), 0, sizeof(cpu_set_t))

typedef struct {
    unsigned long __bits[CPU_SETSIZE/64];
} cpu_set_t;

int pthread_setaffinity_np(pthread_t thread, size_t cpusetsize, const cpu_set_t* cpuset);
int pthread_getaffinity_np(pthread_t thread, size_t cpusetsize, cpu_set_t* cpuset);

/* Non-portable extensions */
int pthread_getattr_np(pthread_t thread, pthread_attr_t* attr);
int pthread_setname_np(pthread_t thread, const char* name);
int pthread_getname_np(pthread_t thread, char* name, size_t len);

/* Static initializers */
#define PTHREAD_MUTEX_INITIALIZER {0, PTHREAD_MUTEX_DEFAULT, PTHREAD_PRIO_NONE, PTHREAD_PROCESS_PRIVATE, PTHREAD_MUTEX_STALLED, 0, 0, 0}
#define PTHREAD_COND_INITIALIZER  {0, PTHREAD_PROCESS_PRIVATE, CLOCK_REALTIME, NULL, 0}
#define PTHREAD_RWLOCK_INITIALIZER {0, PTHREAD_PROCESS_PRIVATE, 0, 0, 0, 0, 0}

#endif /* LIMITLESSOS_PTHREAD_H */