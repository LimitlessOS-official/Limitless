/*
 * futex.c - LimitlessOS Fast Userspace Mutex Implementation
 * 
 * Provides futex system call support for efficient synchronization
 * primitives including mutexes, condition variables, and semaphores.
 */

#include <userspace/futex.h>
#include <kernel/syscall.h>
#include <kernel/timer.h>
#include <kernel/string.h>

/* Futex operations */
#define FUTEX_WAIT              0
#define FUTEX_WAKE              1
#define FUTEX_FD                2
#define FUTEX_REQUEUE           3
#define FUTEX_CMP_REQUEUE       4
#define FUTEX_WAKE_OP           5
#define FUTEX_LOCK_PI           6
#define FUTEX_UNLOCK_PI         7
#define FUTEX_TRYLOCK_PI        8
#define FUTEX_WAIT_BITSET       9
#define FUTEX_WAKE_BITSET       10
#define FUTEX_WAIT_REQUEUE_PI   11
#define FUTEX_CMP_REQUEUE_PI    12

#define FUTEX_PRIVATE_FLAG      128
#define FUTEX_CLOCK_REALTIME    256

/* Atomic operations */
static inline int atomic_load(volatile int* ptr) {
    return __sync_fetch_and_add(ptr, 0);
}

static inline void atomic_store(volatile int* ptr, int value) {
    __sync_lock_test_and_set(ptr, value);
}

static inline int atomic_compare_exchange(volatile int* ptr, int expected, int desired) {
    return __sync_val_compare_and_swap(ptr, expected, desired);
}

static inline int atomic_fetch_add(volatile int* ptr, int value) {
    return __sync_fetch_and_add(ptr, value);
}

static inline int atomic_fetch_sub(volatile int* ptr, int value) {
    return __sync_fetch_and_sub(ptr, value);
}

/* Futex system call wrapper */
long futex(int* uaddr, int futex_op, int val, 
          const struct timespec* timeout, int* uaddr2, int val3) {
    return syscall6(SYS_futex, (long)uaddr, futex_op, val, 
                   (long)timeout, (long)uaddr2, val3);
}

/* Wait on futex */
int futex_wait(int* futex_word, int expected_value, const struct timespec* timeout) {
    return futex(futex_word, FUTEX_WAIT | FUTEX_PRIVATE_FLAG, 
                expected_value, timeout, NULL, 0);
}

/* Wake threads waiting on futex */
int futex_wake(int* futex_word, int num_threads) {
    return futex(futex_word, FUTEX_WAKE | FUTEX_PRIVATE_FLAG, 
                num_threads, NULL, NULL, 0);
}

/* Wake all threads waiting on futex */
int futex_wake_all(int* futex_word) {
    return futex_wake(futex_word, INT_MAX);
}

/* Requeue threads from one futex to another */
int futex_requeue(int* futex_word1, int* futex_word2, int wake_count, int requeue_count) {
    return futex(futex_word1, FUTEX_REQUEUE | FUTEX_PRIVATE_FLAG,
                wake_count, (struct timespec*)requeue_count, futex_word2, 0);
}

/* Compare and requeue */
int futex_cmp_requeue(int* futex_word1, int* futex_word2, int wake_count, 
                     int requeue_count, int expected_value) {
    return futex(futex_word1, FUTEX_CMP_REQUEUE | FUTEX_PRIVATE_FLAG,
                wake_count, (struct timespec*)requeue_count, futex_word2, expected_value);
}

/* Initialize mutex */
int pthread_mutex_init(pthread_mutex_t* mutex, const pthread_mutexattr_t* attr) {
    if (!mutex) {
        return EINVAL;
    }
    
    memset(mutex, 0, sizeof(pthread_mutex_t));
    
    if (attr) {
        mutex->type = attr->type;
        mutex->protocol = attr->protocol;
        mutex->pshared = attr->pshared;
        mutex->robust = attr->robust;
    } else {
        mutex->type = PTHREAD_MUTEX_DEFAULT;
        mutex->protocol = PTHREAD_PRIO_NONE;
        mutex->pshared = PTHREAD_PROCESS_PRIVATE;
        mutex->robust = PTHREAD_MUTEX_STALLED;
    }
    
    atomic_store(&mutex->futex_word, 0);
    mutex->owner = 0;
    mutex->lock_count = 0;
    
    return 0;
}

/* Destroy mutex */
int pthread_mutex_destroy(pthread_mutex_t* mutex) {
    if (!mutex) {
        return EINVAL;
    }
    
    /* Check if mutex is locked */
    if (atomic_load(&mutex->futex_word) != 0) {
        return EBUSY;
    }
    
    memset(mutex, 0, sizeof(pthread_mutex_t));
    return 0;
}

/* Lock mutex */
int pthread_mutex_lock(pthread_mutex_t* mutex) {
    if (!mutex) {
        return EINVAL;
    }
    
    pthread_t current_thread = pthread_self();
    
    /* Handle recursive mutex */
    if (mutex->type == PTHREAD_MUTEX_RECURSIVE) {
        if (mutex->owner == current_thread) {
            if (mutex->lock_count == UINT_MAX) {
                return EAGAIN;  /* Lock count overflow */
            }
            mutex->lock_count++;
            return 0;
        }
    }
    
    /* Handle error checking mutex */
    if (mutex->type == PTHREAD_MUTEX_ERRORCHECK) {
        if (mutex->owner == current_thread) {
            return EDEADLK;  /* Would deadlock */
        }
    }
    
    /* Try to acquire lock atomically */
    int expected = 0;
    if (atomic_compare_exchange(&mutex->futex_word, expected, 1) == expected) {
        /* Successfully acquired lock */
        mutex->owner = current_thread;
        mutex->lock_count = 1;
        return 0;
    }
    
    /* Lock is contended, use futex to wait */
    while (1) {
        /* Set futex word to 2 (contended state) */
        int old_value = atomic_compare_exchange(&mutex->futex_word, 1, 2);
        
        if (old_value == 0) {
            /* Lock became available */
            mutex->owner = current_thread;
            mutex->lock_count = 1;
            return 0;
        }
        
        if (old_value == 1) {
            /* Successfully marked as contended */
            old_value = 2;
        }
        
        /* Wait for futex to become available */
        int ret = futex_wait(&mutex->futex_word, old_value, NULL);
        if (ret < 0 && ret != -EAGAIN && ret != -EINTR) {
            return -ret;
        }
        
        /* Try to acquire lock again */
        old_value = atomic_compare_exchange(&mutex->futex_word, 0, 2);
        if (old_value == 0) {
            mutex->owner = current_thread;
            mutex->lock_count = 1;
            return 0;
        }
    }
}

/* Try to lock mutex without blocking */
int pthread_mutex_trylock(pthread_mutex_t* mutex) {
    if (!mutex) {
        return EINVAL;
    }
    
    pthread_t current_thread = pthread_self();
    
    /* Handle recursive mutex */
    if (mutex->type == PTHREAD_MUTEX_RECURSIVE) {
        if (mutex->owner == current_thread) {
            if (mutex->lock_count == UINT_MAX) {
                return EAGAIN;
            }
            mutex->lock_count++;
            return 0;
        }
    }
    
    /* Handle error checking mutex */
    if (mutex->type == PTHREAD_MUTEX_ERRORCHECK) {
        if (mutex->owner == current_thread) {
            return EDEADLK;
        }
    }
    
    /* Try to acquire lock atomically */
    int expected = 0;
    if (atomic_compare_exchange(&mutex->futex_word, expected, 1) == expected) {
        mutex->owner = current_thread;
        mutex->lock_count = 1;
        return 0;
    }
    
    return EBUSY;
}

/* Unlock mutex */
int pthread_mutex_unlock(pthread_mutex_t* mutex) {
    if (!mutex) {
        return EINVAL;
    }
    
    pthread_t current_thread = pthread_self();
    
    /* Check ownership for error checking and recursive mutexes */
    if (mutex->type == PTHREAD_MUTEX_ERRORCHECK || 
        mutex->type == PTHREAD_MUTEX_RECURSIVE) {
        if (mutex->owner != current_thread) {
            return EPERM;  /* Not owner */
        }
    }
    
    /* Handle recursive mutex */
    if (mutex->type == PTHREAD_MUTEX_RECURSIVE) {
        mutex->lock_count--;
        if (mutex->lock_count > 0) {
            return 0;  /* Still locked by same thread */
        }
    }
    
    mutex->owner = 0;
    mutex->lock_count = 0;
    
    /* Release the lock */
    int old_value = atomic_fetch_sub(&mutex->futex_word, 1);
    
    /* If there were waiters (value was 2), wake them up */
    if (old_value == 2) {
        futex_wake(&mutex->futex_word, 1);
    }
    
    return 0;
}

/* Timed mutex lock */
int pthread_mutex_timedlock(pthread_mutex_t* mutex, const struct timespec* abs_timeout) {
    if (!mutex || !abs_timeout) {
        return EINVAL;
    }
    
    pthread_t current_thread = pthread_self();
    
    /* Handle recursive mutex */
    if (mutex->type == PTHREAD_MUTEX_RECURSIVE) {
        if (mutex->owner == current_thread) {
            if (mutex->lock_count == UINT_MAX) {
                return EAGAIN;
            }
            mutex->lock_count++;
            return 0;
        }
    }
    
    /* Handle error checking mutex */
    if (mutex->type == PTHREAD_MUTEX_ERRORCHECK) {
        if (mutex->owner == current_thread) {
            return EDEADLK;
        }
    }
    
    /* Try to acquire lock immediately */
    int expected = 0;
    if (atomic_compare_exchange(&mutex->futex_word, expected, 1) == expected) {
        mutex->owner = current_thread;
        mutex->lock_count = 1;
        return 0;
    }
    
    /* Convert absolute timeout to relative timeout */
    struct timespec current_time;
    syscall2(SYS_clock_gettime, CLOCK_REALTIME, (long)&current_time);
    
    struct timespec timeout;
    timeout.tv_sec = abs_timeout->tv_sec - current_time.tv_sec;
    timeout.tv_nsec = abs_timeout->tv_nsec - current_time.tv_nsec;
    
    if (timeout.tv_nsec < 0) {
        timeout.tv_sec--;
        timeout.tv_nsec += 1000000000;
    }
    
    if (timeout.tv_sec < 0) {
        return ETIMEDOUT;
    }
    
    /* Wait with timeout */
    while (1) {
        int old_value = atomic_compare_exchange(&mutex->futex_word, 1, 2);
        
        if (old_value == 0) {
            mutex->owner = current_thread;
            mutex->lock_count = 1;
            return 0;
        }
        
        if (old_value == 1) {
            old_value = 2;
        }
        
        int ret = futex_wait(&mutex->futex_word, old_value, &timeout);
        if (ret == -ETIMEDOUT) {
            return ETIMEDOUT;
        }
        if (ret < 0 && ret != -EAGAIN && ret != -EINTR) {
            return -ret;
        }
        
        /* Update remaining timeout */
        struct timespec new_time;
        syscall2(SYS_clock_gettime, CLOCK_REALTIME, (long)&new_time);
        
        timeout.tv_sec = abs_timeout->tv_sec - new_time.tv_sec;
        timeout.tv_nsec = abs_timeout->tv_nsec - new_time.tv_nsec;
        
        if (timeout.tv_nsec < 0) {
            timeout.tv_sec--;
            timeout.tv_nsec += 1000000000;
        }
        
        if (timeout.tv_sec < 0) {
            return ETIMEDOUT;
        }
        
        /* Try to acquire again */
        old_value = atomic_compare_exchange(&mutex->futex_word, 0, 2);
        if (old_value == 0) {
            mutex->owner = current_thread;
            mutex->lock_count = 1;
            return 0;
        }
    }
}

/* Initialize condition variable */
int pthread_cond_init(pthread_cond_t* cond, const pthread_condattr_t* attr) {
    if (!cond) {
        return EINVAL;
    }
    
    memset(cond, 0, sizeof(pthread_cond_t));
    
    if (attr) {
        cond->pshared = attr->pshared;
        cond->clock = attr->clock;
    } else {
        cond->pshared = PTHREAD_PROCESS_PRIVATE;
        cond->clock = CLOCK_REALTIME;
    }
    
    atomic_store(&cond->futex_word, 0);
    cond->mutex = NULL;
    cond->waiters = 0;
    
    return 0;
}

/* Destroy condition variable */
int pthread_cond_destroy(pthread_cond_t* cond) {
    if (!cond) {
        return EINVAL;
    }
    
    if (cond->waiters > 0) {
        return EBUSY;
    }
    
    memset(cond, 0, sizeof(pthread_cond_t));
    return 0;
}

/* Wait on condition variable */
int pthread_cond_wait(pthread_cond_t* cond, pthread_mutex_t* mutex) {
    if (!cond || !mutex) {
        return EINVAL;
    }
    
    /* Associate condition variable with mutex */
    if (cond->mutex == NULL) {
        cond->mutex = mutex;
    } else if (cond->mutex != mutex) {
        return EINVAL;  /* Different mutex */
    }
    
    /* Increment waiter count */
    atomic_fetch_add(&cond->waiters, 1);
    
    /* Get current futex value */
    int futex_val = atomic_load(&cond->futex_word);
    
    /* Unlock mutex */
    int unlock_ret = pthread_mutex_unlock(mutex);
    if (unlock_ret != 0) {
        atomic_fetch_sub(&cond->waiters, 1);
        return unlock_ret;
    }
    
    /* Wait on futex */
    int ret = futex_wait(&cond->futex_word, futex_val, NULL);
    
    /* Decrement waiter count */
    atomic_fetch_sub(&cond->waiters, 1);
    
    /* Reacquire mutex */
    int lock_ret = pthread_mutex_lock(mutex);
    if (lock_ret != 0) {
        return lock_ret;
    }
    
    if (ret < 0 && ret != -EAGAIN && ret != -EINTR) {
        return -ret;
    }
    
    return 0;
}

/* Timed wait on condition variable */
int pthread_cond_timedwait(pthread_cond_t* cond, pthread_mutex_t* mutex,
                          const struct timespec* abs_timeout) {
    if (!cond || !mutex || !abs_timeout) {
        return EINVAL;
    }
    
    /* Associate condition variable with mutex */
    if (cond->mutex == NULL) {
        cond->mutex = mutex;
    } else if (cond->mutex != mutex) {
        return EINVAL;
    }
    
    /* Convert absolute to relative timeout */
    struct timespec current_time;
    syscall2(SYS_clock_gettime, cond->clock, (long)&current_time);
    
    struct timespec timeout;
    timeout.tv_sec = abs_timeout->tv_sec - current_time.tv_sec;
    timeout.tv_nsec = abs_timeout->tv_nsec - current_time.tv_nsec;
    
    if (timeout.tv_nsec < 0) {
        timeout.tv_sec--;
        timeout.tv_nsec += 1000000000;
    }
    
    if (timeout.tv_sec < 0) {
        return ETIMEDOUT;
    }
    
    /* Increment waiter count */
    atomic_fetch_add(&cond->waiters, 1);
    
    /* Get current futex value */
    int futex_val = atomic_load(&cond->futex_word);
    
    /* Unlock mutex */
    int unlock_ret = pthread_mutex_unlock(mutex);
    if (unlock_ret != 0) {
        atomic_fetch_sub(&cond->waiters, 1);
        return unlock_ret;
    }
    
    /* Wait on futex with timeout */
    int ret = futex_wait(&cond->futex_word, futex_val, &timeout);
    
    /* Decrement waiter count */
    atomic_fetch_sub(&cond->waiters, 1);
    
    /* Reacquire mutex */
    int lock_ret = pthread_mutex_lock(mutex);
    if (lock_ret != 0) {
        return lock_ret;
    }
    
    if (ret == -ETIMEDOUT) {
        return ETIMEDOUT;
    }
    
    if (ret < 0 && ret != -EAGAIN && ret != -EINTR) {
        return -ret;
    }
    
    return 0;
}

/* Signal condition variable */
int pthread_cond_signal(pthread_cond_t* cond) {
    if (!cond) {
        return EINVAL;
    }
    
    /* Only signal if there are waiters */
    if (atomic_load(&cond->waiters) > 0) {
        atomic_fetch_add(&cond->futex_word, 1);
        futex_wake(&cond->futex_word, 1);
    }
    
    return 0;
}

/* Broadcast condition variable */
int pthread_cond_broadcast(pthread_cond_t* cond) {
    if (!cond) {
        return EINVAL;
    }
    
    int waiters = atomic_load(&cond->waiters);
    if (waiters > 0) {
        atomic_fetch_add(&cond->futex_word, 1);
        futex_wake(&cond->futex_word, waiters);
    }
    
    return 0;
}