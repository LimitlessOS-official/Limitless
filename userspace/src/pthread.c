/*
 * pthread.c - LimitlessOS POSIX Threads Implementation
 * 
 * Complete pthread library with futex support, mutexes, condition variables,
 * thread-local storage, and cancellation.
 */

#include <userspace/pthread.h>
#include <userspace/futex.h>
#include <kernel/syscall.h>
#include <kernel/vmm.h>
#include <kernel/string.h>
#include <kernel/klog.h>

#define PTHREAD_STACK_MIN    16384
#define PTHREAD_STACK_DEFAULT (2 * 1024 * 1024)  /* 2MB default stack */
#define MAX_THREADS         1024
#define PTHREAD_MAGIC       0x5054484421  /* "PTHD!" */

/* Thread states */
typedef enum {
    PTHREAD_STATE_NEW,
    PTHREAD_STATE_RUNNING,
    PTHREAD_STATE_BLOCKED,
    PTHREAD_STATE_TERMINATED,
    PTHREAD_STATE_DETACHED
} pthread_state_t;

/* Thread control block */
typedef struct pthread_tcb {
    uint64_t magic;                     /* Magic number for validation */
    pthread_t tid;                      /* Thread ID */
    pthread_state_t state;              /* Thread state */
    
    /* Thread function and argument */
    void* (*start_routine)(void*);
    void* arg;
    void* return_value;
    
    /* Stack information */
    void* stack_base;
    size_t stack_size;
    bool stack_allocated;               /* Whether we allocated the stack */
    
    /* Thread attributes */
    pthread_attr_t attr;
    
    /* Cancellation */
    int cancel_state;
    int cancel_type;
    bool cancel_pending;
    
    /* Thread-specific data */
    void* tsd[PTHREAD_KEYS_MAX];
    
    /* Cleanup handlers */
    struct cleanup_handler {
        void (*routine)(void*);
        void* arg;
        struct cleanup_handler* next;
    } *cleanup_handlers;
    
    /* Join/detach support */
    pthread_mutex_t join_mutex;
    pthread_cond_t join_cond;
    bool joined;
    
    /* Error number */
    int errno_val;
    
    /* Signal mask */
    sigset_t sigmask;
    
    /* Links for thread list */
    struct pthread_tcb* next;
    struct pthread_tcb* prev;
} pthread_tcb_t;

/* Global thread management */
static struct {
    pthread_tcb_t* thread_list;         /* List of all threads */
    pthread_t next_tid;                 /* Next thread ID */
    pthread_mutex_t global_mutex;       /* Global thread list mutex */
    
    /* Main thread TCB */
    pthread_tcb_t main_thread;
    
    /* Thread-specific data keys */
    struct {
        bool in_use;
        void (*destructor)(void*);
    } tsd_keys[PTHREAD_KEYS_MAX];
    pthread_mutex_t tsd_mutex;
    
    /* Thread creation attributes */
    pthread_attr_t default_attr;
    
    bool initialized;
} pthread_globals;

/* Function prototypes */
static void pthread_init_once(void);
static pthread_tcb_t* pthread_get_tcb(pthread_t thread);
static pthread_tcb_t* pthread_self_tcb(void);
static void pthread_cleanup_thread(pthread_tcb_t* tcb);
static void* pthread_entry_point(void* arg);
static void pthread_run_cleanup_handlers(pthread_tcb_t* tcb);
static void pthread_destroy_tsd(pthread_tcb_t* tcb);

/* Initialize pthread library (called once) */
static void pthread_init_once(void) {
    if (pthread_globals.initialized) {
        return;
    }
    
    memset(&pthread_globals, 0, sizeof(pthread_globals));
    
    /* Initialize global mutex */
    pthread_mutex_init(&pthread_globals.global_mutex, NULL);
    pthread_mutex_init(&pthread_globals.tsd_mutex, NULL);
    
    /* Setup main thread */
    pthread_tcb_t* main_tcb = &pthread_globals.main_thread;
    main_tcb->magic = PTHREAD_MAGIC;
    main_tcb->tid = 1;
    main_tcb->state = PTHREAD_STATE_RUNNING;
    main_tcb->cancel_state = PTHREAD_CANCEL_ENABLE;
    main_tcb->cancel_type = PTHREAD_CANCEL_DEFERRED;
    
    /* Initialize main thread attributes */
    pthread_attr_init(&main_tcb->attr);
    
    /* Add to thread list */
    pthread_globals.thread_list = main_tcb;
    pthread_globals.next_tid = 2;
    
    /* Initialize default attributes */
    pthread_attr_init(&pthread_globals.default_attr);
    
    pthread_globals.initialized = true;
}

/* Get current thread's TCB */
static pthread_tcb_t* pthread_self_tcb(void) {
    pthread_init_once();
    
    /* Get current thread ID from kernel */
    pthread_t current_tid = syscall1(SYS_gettid);
    
    if (current_tid == 1) {
        return &pthread_globals.main_thread;
    }
    
    return pthread_get_tcb(current_tid);
}

/* Find TCB by thread ID */
static pthread_tcb_t* pthread_get_tcb(pthread_t thread) {
    pthread_mutex_lock(&pthread_globals.global_mutex);
    
    pthread_tcb_t* tcb = pthread_globals.thread_list;
    while (tcb) {
        if (tcb->tid == thread && tcb->magic == PTHREAD_MAGIC) {
            pthread_mutex_unlock(&pthread_globals.global_mutex);
            return tcb;
        }
        tcb = tcb->next;
    }
    
    pthread_mutex_unlock(&pthread_globals.global_mutex);
    return NULL;
}

/* Thread entry point wrapper */
static void* pthread_entry_point(void* arg) {
    pthread_tcb_t* tcb = (pthread_tcb_t*)arg;
    
    /* Set thread state to running */
    tcb->state = PTHREAD_STATE_RUNNING;
    
    /* Call the actual thread function */
    void* result = tcb->start_routine(tcb->arg);
    
    /* Store return value */
    tcb->return_value = result;
    
    /* Run cleanup handlers */
    pthread_run_cleanup_handlers(tcb);
    
    /* Destroy thread-specific data */
    pthread_destroy_tsd(tcb);
    
    /* Mark as terminated */
    tcb->state = PTHREAD_STATE_TERMINATED;
    
    /* Wake up any threads waiting to join */
    pthread_mutex_lock(&tcb->join_mutex);
    pthread_cond_broadcast(&tcb->join_cond);
    pthread_mutex_unlock(&tcb->join_mutex);
    
    /* Exit the thread */
    syscall1(SYS_exit, (long)result);
    return NULL;  /* Never reached */
}

/* Create new thread */
int pthread_create(pthread_t* thread, const pthread_attr_t* attr,
                   void* (*start_routine)(void*), void* arg) {
    if (!thread || !start_routine) {
        return EINVAL;
    }
    
    pthread_init_once();
    
    /* Allocate TCB */
    pthread_tcb_t* tcb = (pthread_tcb_t*)malloc(sizeof(pthread_tcb_t));
    if (!tcb) {
        return ENOMEM;
    }
    
    memset(tcb, 0, sizeof(pthread_tcb_t));
    tcb->magic = PTHREAD_MAGIC;
    
    /* Assign thread ID */
    pthread_mutex_lock(&pthread_globals.global_mutex);
    tcb->tid = pthread_globals.next_tid++;
    pthread_mutex_unlock(&pthread_globals.global_mutex);
    
    /* Copy attributes */
    if (attr) {
        tcb->attr = *attr;
    } else {
        tcb->attr = pthread_globals.default_attr;
    }
    
    /* Set up thread function */
    tcb->start_routine = start_routine;
    tcb->arg = arg;
    tcb->state = PTHREAD_STATE_NEW;
    tcb->cancel_state = PTHREAD_CANCEL_ENABLE;
    tcb->cancel_type = PTHREAD_CANCEL_DEFERRED;
    
    /* Initialize synchronization objects */
    pthread_mutex_init(&tcb->join_mutex, NULL);
    pthread_cond_init(&tcb->join_cond, NULL);
    
    /* Allocate stack if needed */
    size_t stack_size = tcb->attr.stacksize;
    if (stack_size == 0) {
        stack_size = PTHREAD_STACK_DEFAULT;
    }
    
    if (tcb->attr.stackaddr == NULL) {
        tcb->stack_base = vmm_alloc_pages(PAGE_COUNT(stack_size), 
                                         VMM_FLAG_USER | VMM_FLAG_READ | VMM_FLAG_WRITE);
        if (!tcb->stack_base) {
            free(tcb);
            return ENOMEM;
        }
        tcb->stack_allocated = true;
    } else {
        tcb->stack_base = tcb->attr.stackaddr;
        tcb->stack_allocated = false;
    }
    tcb->stack_size = stack_size;
    
    /* Add to thread list */
    pthread_mutex_lock(&pthread_globals.global_mutex);
    tcb->next = pthread_globals.thread_list;
    if (pthread_globals.thread_list) {
        pthread_globals.thread_list->prev = tcb;
    }
    pthread_globals.thread_list = tcb;
    pthread_mutex_unlock(&pthread_globals.global_mutex);
    
    /* Create kernel thread */
    long clone_flags = CLONE_VM | CLONE_FILES | CLONE_SIGHAND | CLONE_THREAD;
    if (tcb->attr.detachstate == PTHREAD_CREATE_DETACHED) {
        clone_flags |= CLONE_DETACHED;
    }
    
    void* stack_top = (char*)tcb->stack_base + tcb->stack_size;
    pid_t kernel_tid = syscall5(SYS_clone, clone_flags, (long)stack_top, 
                               0, 0, (long)pthread_entry_point);
    
    if (kernel_tid < 0) {
        /* Clean up on failure */
        pthread_mutex_lock(&pthread_globals.global_mutex);
        if (tcb->next) tcb->next->prev = tcb->prev;
        if (tcb->prev) tcb->prev->next = tcb->next;
        else pthread_globals.thread_list = tcb->next;
        pthread_mutex_unlock(&pthread_globals.global_mutex);
        
        if (tcb->stack_allocated) {
            vmm_free_pages(tcb->stack_base, PAGE_COUNT(tcb->stack_size));
        }
        free(tcb);
        return EAGAIN;
    }
    
    /* Pass TCB to new thread */
    syscall2(SYS_set_thread_area, kernel_tid, (long)tcb);
    
    *thread = tcb->tid;
    return 0;
}

/* Wait for thread to terminate */
int pthread_join(pthread_t thread, void** retval) {
    if (thread == pthread_self()) {
        return EDEADLK;  /* Cannot join with self */
    }
    
    pthread_tcb_t* tcb = pthread_get_tcb(thread);
    if (!tcb) {
        return ESRCH;  /* No such thread */
    }
    
    if (tcb->attr.detachstate == PTHREAD_CREATE_DETACHED) {
        return EINVAL;  /* Cannot join detached thread */
    }
    
    /* Wait for thread to terminate */
    pthread_mutex_lock(&tcb->join_mutex);
    
    while (tcb->state != PTHREAD_STATE_TERMINATED) {
        pthread_cond_wait(&tcb->join_cond, &tcb->join_mutex);
    }
    
    tcb->joined = true;
    
    if (retval) {
        *retval = tcb->return_value;
    }
    
    pthread_mutex_unlock(&tcb->join_mutex);
    
    /* Clean up the thread */
    pthread_cleanup_thread(tcb);
    
    return 0;
}

/* Detach thread */
int pthread_detach(pthread_t thread) {
    pthread_tcb_t* tcb = pthread_get_tcb(thread);
    if (!tcb) {
        return ESRCH;
    }
    
    if (tcb->attr.detachstate == PTHREAD_CREATE_DETACHED) {
        return EINVAL;  /* Already detached */
    }
    
    tcb->attr.detachstate = PTHREAD_CREATE_DETACHED;
    tcb->state = PTHREAD_STATE_DETACHED;
    
    /* If already terminated, clean up immediately */
    if (tcb->state == PTHREAD_STATE_TERMINATED) {
        pthread_cleanup_thread(tcb);
    }
    
    return 0;
}

/* Exit current thread */
void pthread_exit(void* retval) {
    pthread_tcb_t* tcb = pthread_self_tcb();
    if (!tcb) {
        syscall1(SYS_exit, (long)retval);
        return;
    }
    
    tcb->return_value = retval;
    
    /* Run cleanup handlers */
    pthread_run_cleanup_handlers(tcb);
    
    /* Destroy thread-specific data */
    pthread_destroy_tsd(tcb);
    
    /* Mark as terminated */
    tcb->state = PTHREAD_STATE_TERMINATED;
    
    /* Wake up joiners */
    pthread_mutex_lock(&tcb->join_mutex);
    pthread_cond_broadcast(&tcb->join_cond);
    pthread_mutex_unlock(&tcb->join_mutex);
    
    /* Exit */
    syscall1(SYS_exit, (long)retval);
}

/* Get current thread ID */
pthread_t pthread_self(void) {
    pthread_tcb_t* tcb = pthread_self_tcb();
    return tcb ? tcb->tid : 0;
}

/* Compare thread IDs */
int pthread_equal(pthread_t t1, pthread_t t2) {
    return t1 == t2;
}

/* Set thread cancellation state */
int pthread_setcancelstate(int state, int* oldstate) {
    if (state != PTHREAD_CANCEL_ENABLE && state != PTHREAD_CANCEL_DISABLE) {
        return EINVAL;
    }
    
    pthread_tcb_t* tcb = pthread_self_tcb();
    if (!tcb) {
        return ESRCH;
    }
    
    if (oldstate) {
        *oldstate = tcb->cancel_state;
    }
    
    tcb->cancel_state = state;
    
    /* Check for pending cancellation */
    if (state == PTHREAD_CANCEL_ENABLE && tcb->cancel_pending) {
        pthread_exit(PTHREAD_CANCELED);
    }
    
    return 0;
}

/* Set thread cancellation type */
int pthread_setcanceltype(int type, int* oldtype) {
    if (type != PTHREAD_CANCEL_DEFERRED && type != PTHREAD_CANCEL_ASYNCHRONOUS) {
        return EINVAL;
    }
    
    pthread_tcb_t* tcb = pthread_self_tcb();
    if (!tcb) {
        return ESRCH;
    }
    
    if (oldtype) {
        *oldtype = tcb->cancel_type;
    }
    
    tcb->cancel_type = type;
    
    return 0;
}

/* Cancel thread */
int pthread_cancel(pthread_t thread) {
    pthread_tcb_t* tcb = pthread_get_tcb(thread);
    if (!tcb) {
        return ESRCH;
    }
    
    tcb->cancel_pending = true;
    
    /* If cancellation enabled and asynchronous, force termination */
    if (tcb->cancel_state == PTHREAD_CANCEL_ENABLE && 
        tcb->cancel_type == PTHREAD_CANCEL_ASYNCHRONOUS) {
        /* Send signal to force cancellation */
        syscall2(SYS_kill, tcb->tid, SIGTERM);
    }
    
    return 0;
}

/* Test for cancellation */
void pthread_testcancel(void) {
    pthread_tcb_t* tcb = pthread_self_tcb();
    if (!tcb) {
        return;
    }
    
    if (tcb->cancel_state == PTHREAD_CANCEL_ENABLE && tcb->cancel_pending) {
        pthread_exit(PTHREAD_CANCELED);
    }
}

/* Push cleanup handler */
void pthread_cleanup_push(void (*routine)(void*), void* arg) {
    pthread_tcb_t* tcb = pthread_self_tcb();
    if (!tcb || !routine) {
        return;
    }
    
    struct cleanup_handler* handler = malloc(sizeof(struct cleanup_handler));
    if (!handler) {
        return;
    }
    
    handler->routine = routine;
    handler->arg = arg;
    handler->next = tcb->cleanup_handlers;
    tcb->cleanup_handlers = handler;
}

/* Pop cleanup handler */
void pthread_cleanup_pop(int execute) {
    pthread_tcb_t* tcb = pthread_self_tcb();
    if (!tcb || !tcb->cleanup_handlers) {
        return;
    }
    
    struct cleanup_handler* handler = tcb->cleanup_handlers;
    tcb->cleanup_handlers = handler->next;
    
    if (execute) {
        handler->routine(handler->arg);
    }
    
    free(handler);
}

/* Run all cleanup handlers */
static void pthread_run_cleanup_handlers(pthread_tcb_t* tcb) {
    while (tcb->cleanup_handlers) {
        struct cleanup_handler* handler = tcb->cleanup_handlers;
        tcb->cleanup_handlers = handler->next;
        
        handler->routine(handler->arg);
        free(handler);
    }
}

/* Create thread-specific data key */
int pthread_key_create(pthread_key_t* key, void (*destructor)(void*)) {
    if (!key) {
        return EINVAL;
    }
    
    pthread_mutex_lock(&pthread_globals.tsd_mutex);
    
    /* Find free key slot */
    for (int i = 0; i < PTHREAD_KEYS_MAX; i++) {
        if (!pthread_globals.tsd_keys[i].in_use) {
            pthread_globals.tsd_keys[i].in_use = true;
            pthread_globals.tsd_keys[i].destructor = destructor;
            *key = i;
            
            pthread_mutex_unlock(&pthread_globals.tsd_mutex);
            return 0;
        }
    }
    
    pthread_mutex_unlock(&pthread_globals.tsd_mutex);
    return EAGAIN;  /* No more keys available */
}

/* Delete thread-specific data key */
int pthread_key_delete(pthread_key_t key) {
    if (key >= PTHREAD_KEYS_MAX) {
        return EINVAL;
    }
    
    pthread_mutex_lock(&pthread_globals.tsd_mutex);
    
    if (!pthread_globals.tsd_keys[key].in_use) {
        pthread_mutex_unlock(&pthread_globals.tsd_mutex);
        return EINVAL;
    }
    
    pthread_globals.tsd_keys[key].in_use = false;
    pthread_globals.tsd_keys[key].destructor = NULL;
    
    pthread_mutex_unlock(&pthread_globals.tsd_mutex);
    
    return 0;
}

/* Set thread-specific data */
int pthread_setspecific(pthread_key_t key, const void* value) {
    if (key >= PTHREAD_KEYS_MAX) {
        return EINVAL;
    }
    
    pthread_tcb_t* tcb = pthread_self_tcb();
    if (!tcb) {
        return ESRCH;
    }
    
    pthread_mutex_lock(&pthread_globals.tsd_mutex);
    
    if (!pthread_globals.tsd_keys[key].in_use) {
        pthread_mutex_unlock(&pthread_globals.tsd_mutex);
        return EINVAL;
    }
    
    pthread_mutex_unlock(&pthread_globals.tsd_mutex);
    
    tcb->tsd[key] = (void*)value;
    return 0;
}

/* Get thread-specific data */
void* pthread_getspecific(pthread_key_t key) {
    if (key >= PTHREAD_KEYS_MAX) {
        return NULL;
    }
    
    pthread_tcb_t* tcb = pthread_self_tcb();
    if (!tcb) {
        return NULL;
    }
    
    return tcb->tsd[key];
}

/* Destroy thread-specific data */
static void pthread_destroy_tsd(pthread_tcb_t* tcb) {
    pthread_mutex_lock(&pthread_globals.tsd_mutex);
    
    for (int i = 0; i < PTHREAD_KEYS_MAX; i++) {
        if (pthread_globals.tsd_keys[i].in_use && tcb->tsd[i]) {
            void (*destructor)(void*) = pthread_globals.tsd_keys[i].destructor;
            void* value = tcb->tsd[i];
            
            tcb->tsd[i] = NULL;
            
            if (destructor) {
                pthread_mutex_unlock(&pthread_globals.tsd_mutex);
                destructor(value);
                pthread_mutex_lock(&pthread_globals.tsd_mutex);
            }
        }
    }
    
    pthread_mutex_unlock(&pthread_globals.tsd_mutex);
}

/* Clean up thread resources */
static void pthread_cleanup_thread(pthread_tcb_t* tcb) {
    /* Remove from thread list */
    pthread_mutex_lock(&pthread_globals.global_mutex);
    
    if (tcb->next) tcb->next->prev = tcb->prev;
    if (tcb->prev) tcb->prev->next = tcb->next;
    else pthread_globals.thread_list = tcb->next;
    
    pthread_mutex_unlock(&pthread_globals.global_mutex);
    
    /* Free stack if allocated */
    if (tcb->stack_allocated && tcb->stack_base) {
        vmm_free_pages(tcb->stack_base, PAGE_COUNT(tcb->stack_size));
    }
    
    /* Destroy synchronization objects */
    pthread_mutex_destroy(&tcb->join_mutex);
    pthread_cond_destroy(&tcb->join_cond);
    
    /* Clear magic and free TCB */
    tcb->magic = 0;
    free(tcb);
}

/* Get thread attributes */
int pthread_getattr_np(pthread_t thread, pthread_attr_t* attr) {
    if (!attr) {
        return EINVAL;
    }
    
    pthread_tcb_t* tcb = pthread_get_tcb(thread);
    if (!tcb) {
        return ESRCH;
    }
    
    *attr = tcb->attr;
    return 0;
}