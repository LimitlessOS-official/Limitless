#ifndef _LINUX_ATOMIC_H
#define _LINUX_ATOMIC_H

// This is a dummy implementation for compilation purposes.
// A real implementation would use architecture-specific atomic operations.

typedef struct {
    volatile int counter;
} atomic_t;

#define ATOMIC_INIT(i)  { (i) }

static inline int atomic_read(const atomic_t *v)
{
    return v->counter;
}

static inline void atomic_set(atomic_t *v, int i)
{
    v->counter = i;
}

static inline void atomic_inc(atomic_t *v)
{
    __sync_fetch_and_add(&v->counter, 1);
}

static inline void atomic_dec(atomic_t *v)
{
    __sync_fetch_and_sub(&v->counter, 1);
}

static inline int atomic_dec_and_test(atomic_t *v)
{
    return __sync_sub_and_fetch(&v->counter, 1) == 0;
}

#endif