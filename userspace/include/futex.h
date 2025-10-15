/*
 * futex.h - LimitlessOS Fast Userspace Mutex Header
 * 
 * Provides futex system call interface for efficient synchronization.
 */

#ifndef LIMITLESSOS_FUTEX_H
#define LIMITLESSOS_FUTEX_H

#include <kernel/types.h>
#include <kernel/time.h>

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

/* Error codes */
#define EAGAIN          11
#define EINVAL          22
#define ETIMEDOUT       110
#define EWOULDBLOCK     EAGAIN

/* Futex system call */
long futex(int* uaddr, int futex_op, int val, 
          const struct timespec* timeout, int* uaddr2, int val3);

/* High-level futex operations */
int futex_wait(int* futex_word, int expected_value, const struct timespec* timeout);
int futex_wake(int* futex_word, int num_threads);
int futex_wake_all(int* futex_word);
int futex_requeue(int* futex_word1, int* futex_word2, int wake_count, int requeue_count);
int futex_cmp_requeue(int* futex_word1, int* futex_word2, int wake_count, 
                     int requeue_count, int expected_value);

#endif /* LIMITLESSOS_FUTEX_H */