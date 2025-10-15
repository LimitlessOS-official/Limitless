#ifndef _LINUX_SPINLOCK_H
#define _LINUX_SPINLOCK_H

// This is a dummy implementation for compilation purposes.
// A real implementation would use architecture-specific atomic operations.

typedef struct {
    volatile int lock;
} spinlock_t;

#define SPIN_LOCK_UNLOCKED { 0 }

static inline void spin_lock_init(spinlock_t *sl)
{
    sl->lock = 0;
}

static inline void spin_lock(spinlock_t *sl)
{
    // In a real kernel, this would be an atomic test-and-set loop.
    while (__sync_lock_test_and_set(&sl->lock, 1)) {
        // Busy-wait
    }
}

static inline void spin_unlock(spinlock_t *sl)
{
    __sync_lock_release(&sl->lock);
}

#endif