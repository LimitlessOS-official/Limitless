/*
 * Pipe Implementation
 * 
 * UNIX-style pipes for inter-process communication.
 * 
 * Copyright (c) 2024 LimitlessOS Project
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

typedef long ssize_t;

#define PIPE_BUF_SIZE 4096
#define MAX_PIPES 128

typedef struct {
    uint8_t buffer[PIPE_BUF_SIZE];
    size_t read_pos;
    size_t write_pos;
    size_t count;
    bool in_use;
    int read_fd;
    int write_fd;
} pipe_t;

static pipe_t pipe_table[MAX_PIPES];

// Initialize pipe system
void pipe_init(void) {
    for (int i = 0; i < MAX_PIPES; i++) {
        pipe_table[i].in_use = false;
        pipe_table[i].count = 0;
        pipe_table[i].read_pos = 0;
        pipe_table[i].write_pos = 0;
    }
}

// Allocate a pipe
static pipe_t *pipe_alloc(void) {
    for (int i = 0; i < MAX_PIPES; i++) {
        if (!pipe_table[i].in_use) {
            pipe_table[i].in_use = true;
            pipe_table[i].count = 0;
            pipe_table[i].read_pos = 0;
            pipe_table[i].write_pos = 0;
            return &pipe_table[i];
        }
    }
    return NULL;
}

// Create a pipe
int sys_pipe(int pipefd[2]) {
    if (!pipefd) {
        return -1;
    }
    
    pipe_t *pipe = pipe_alloc();
    if (!pipe) {
        return -1;  // Too many pipes
    }
    
    // TODO: Allocate file descriptors properly
    // For now, use simple indices
    pipefd[0] = (pipe - pipe_table) * 2;      // Read end
    pipefd[1] = (pipe - pipe_table) * 2 + 1;  // Write end
    
    pipe->read_fd = pipefd[0];
    pipe->write_fd = pipefd[1];
    
    return 0;
}

// Write to pipe
ssize_t pipe_write(int fd, const void *buf, size_t count) {
    // Find pipe from file descriptor
    int pipe_idx = fd / 2;
    if (pipe_idx < 0 || pipe_idx >= MAX_PIPES) {
        return -1;
    }
    
    pipe_t *pipe = &pipe_table[pipe_idx];
    if (!pipe->in_use || fd != pipe->write_fd) {
        return -1;
    }
    
    const uint8_t *data = (const uint8_t *)buf;
    size_t written = 0;
    
    while (written < count && pipe->count < PIPE_BUF_SIZE) {
        pipe->buffer[pipe->write_pos] = data[written];
        pipe->write_pos = (pipe->write_pos + 1) % PIPE_BUF_SIZE;
        pipe->count++;
        written++;
    }
    
    return written;
}

// Read from pipe
ssize_t pipe_read(int fd, void *buf, size_t count) {
    // Find pipe from file descriptor
    int pipe_idx = fd / 2;
    if (pipe_idx < 0 || pipe_idx >= MAX_PIPES) {
        return -1;
    }
    
    pipe_t *pipe = &pipe_table[pipe_idx];
    if (!pipe->in_use || fd != pipe->read_fd) {
        return -1;
    }
    
    uint8_t *data = (uint8_t *)buf;
    size_t read_count = 0;
    
    while (read_count < count && pipe->count > 0) {
        data[read_count] = pipe->buffer[pipe->read_pos];
        pipe->read_pos = (pipe->read_pos + 1) % PIPE_BUF_SIZE;
        pipe->count--;
        read_count++;
    }
    
    return read_count;
}

// Close pipe end
int pipe_close(int fd) {
    int pipe_idx = fd / 2;
    if (pipe_idx < 0 || pipe_idx >= MAX_PIPES) {
        return -1;
    }
    
    pipe_t *pipe = &pipe_table[pipe_idx];
    if (!pipe->in_use) {
        return -1;
    }
    
    // TODO: Track which end is closed
    // For now, just mark as unused
    pipe->in_use = false;
    
    return 0;
}

/*
 * Shared Memory Implementation
 */

#define MAX_SHM_SEGMENTS 64
#define SHM_SIZE (1024 * 1024)  // 1MB per segment

typedef struct {
    int key;
    size_t size;
    void *addr;
    int attach_count;
    bool in_use;
} shm_segment_t;

static shm_segment_t shm_table[MAX_SHM_SEGMENTS];

// Initialize shared memory system
void shm_init(void) {
    for (int i = 0; i < MAX_SHM_SEGMENTS; i++) {
        shm_table[i].in_use = false;
        shm_table[i].attach_count = 0;
    }
}

// Create/get shared memory segment
int sys_shmget(int key, size_t size, int shmflg) {
    // Look for existing segment with this key
    if (key != 0) {  // IPC_PRIVATE
        for (int i = 0; i < MAX_SHM_SEGMENTS; i++) {
            if (shm_table[i].in_use && shm_table[i].key == key) {
                return i;  // Return segment ID
            }
        }
    }
    
    // Create new segment
    for (int i = 0; i < MAX_SHM_SEGMENTS; i++) {
        if (!shm_table[i].in_use) {
            // TODO: Allocate memory for segment
            // For now, use a placeholder
            shm_table[i].key = key;
            shm_table[i].size = size;
            shm_table[i].addr = NULL;  // Would be allocated from kernel heap
            shm_table[i].attach_count = 0;
            shm_table[i].in_use = true;
            
            return i;  // Return segment ID
        }
    }
    
    return -1;  // No free segments
}

// Attach shared memory segment
void *sys_shmat(int shmid, const void *shmaddr, int shmflg) {
    if (shmid < 0 || shmid >= MAX_SHM_SEGMENTS) {
        return (void *)-1;
    }
    
    shm_segment_t *seg = &shm_table[shmid];
    if (!seg->in_use) {
        return (void *)-1;
    }
    
    // TODO: Map segment into process address space
    seg->attach_count++;
    
    return seg->addr;  // Return address of segment
}

// Detach shared memory segment
int sys_shmdt(const void *shmaddr) {
    // Find segment by address
    for (int i = 0; i < MAX_SHM_SEGMENTS; i++) {
        if (shm_table[i].in_use && shm_table[i].addr == shmaddr) {
            if (shm_table[i].attach_count > 0) {
                shm_table[i].attach_count--;
            }
            return 0;
        }
    }
    
    return -1;
}

/*
 * Semaphore Implementation
 */

#define MAX_SEM_SETS 64
#define MAX_SEMS_PER_SET 32

typedef struct {
    int key;
    int nsems;
    int values[MAX_SEMS_PER_SET];
    bool in_use;
} sem_set_t;

static sem_set_t sem_table[MAX_SEM_SETS];

// Initialize semaphore system
void sem_init(void) {
    for (int i = 0; i < MAX_SEM_SETS; i++) {
        sem_table[i].in_use = false;
    }
}

// Create/get semaphore set
int sys_semget(int key, int nsems, int semflg) {
    if (nsems < 1 || nsems > MAX_SEMS_PER_SET) {
        return -1;
    }
    
    // Look for existing set
    if (key != 0) {
        for (int i = 0; i < MAX_SEM_SETS; i++) {
            if (sem_table[i].in_use && sem_table[i].key == key) {
                return i;
            }
        }
    }
    
    // Create new set
    for (int i = 0; i < MAX_SEM_SETS; i++) {
        if (!sem_table[i].in_use) {
            sem_table[i].key = key;
            sem_table[i].nsems = nsems;
            sem_table[i].in_use = true;
            
            // Initialize all semaphores to 0
            for (int j = 0; j < nsems; j++) {
                sem_table[i].values[j] = 0;
            }
            
            return i;
        }
    }
    
    return -1;
}

// Semaphore operation
int sys_semop(int semid, void *sops, size_t nsops) {
    if (semid < 0 || semid >= MAX_SEM_SETS) {
        return -1;
    }
    
    sem_set_t *set = &sem_table[semid];
    if (!set->in_use) {
        return -1;
    }
    
    // TODO: Implement atomic semaphore operations
    // For now, just a placeholder
    
    return 0;
}

// Initialize all IPC subsystems
void ipc_init(void) {
    pipe_init();
    shm_init();
    sem_init();
}
