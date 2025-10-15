/**
 * Inter-Process Communication (IPC) System for LimitlessOS
 * Provides message queues, shared memory, semaphores, and pipes
 */

#pragma once
#include "kernel.h"
#include "process.h"

/* IPC Types */
typedef enum {
    IPC_MESSAGE_QUEUE = 0,
    IPC_SHARED_MEMORY,
    IPC_SEMAPHORE,
    IPC_PIPE,
    IPC_SOCKET_PAIR,
    IPC_EVENT_QUEUE
} ipc_type_t;

/* IPC Message Structure */
typedef struct ipc_message {
    uint32_t type;          /* Message type */
    pid_t sender_pid;       /* Sender process ID */
    uint32_t priority;      /* Message priority */
    size_t data_size;       /* Data size */
    void* data;             /* Message data */
    uint64_t timestamp;     /* Message timestamp */
    struct ipc_message* next; /* Queue link */
} ipc_message_t;

/* Message Queue */
typedef struct message_queue {
    uint32_t queue_id;
    pid_t owner_pid;
    uint32_t max_messages;
    uint32_t max_message_size;
    uint32_t current_count;
    
    ipc_message_t* head;
    ipc_message_t* tail;
    
    spinlock_t lock;
    struct wait_queue* readers;
    struct wait_queue* writers;
} message_queue_t;

/* Shared Memory Segment */
typedef struct shared_memory {
    uint32_t segment_id;
    size_t size;
    void* virtual_addr;
    uint32_t permissions;
    
    pid_t creator_pid;
    uint32_t attach_count;
    pid_t* attached_pids;
    
    spinlock_t lock;
} shared_memory_t;

/* Semaphore */
typedef struct semaphore {
    uint32_t semaphore_id;
    int32_t value;
    int32_t max_value;
    
    pid_t owner_pid;
    struct wait_queue* waiters;
    
    spinlock_t lock;
} semaphore_t;

/* Pipe */
typedef struct pipe {
    uint32_t pipe_id;
    size_t buffer_size;
    uint8_t* buffer;
    size_t read_pos;
    size_t write_pos;
    size_t data_count;
    
    bool read_end_open;
    bool write_end_open;
    
    struct wait_queue* readers;
    struct wait_queue* writers;
    
    spinlock_t lock;
} pipe_t;

/* IPC Channel (unified interface) */
typedef struct ipc_channel {
    uint32_t channel_id;
    ipc_type_t type;
    pid_t creator_pid;
    pid_t peer_pid;
    
    union {
        message_queue_t* mq;
        shared_memory_t* shm;
        semaphore_t* sem;
        pipe_t* pipe;
    } handle;
    
    bool in_use;
    spinlock_t lock;
} ipc_channel_t;

/* IPC System State */
typedef struct ipc_system {
    ipc_channel_t channels[MAX_IPC_CHANNELS];
    uint32_t next_channel_id;
    spinlock_t global_lock;
    
    /* Statistics */
    uint64_t messages_sent;
    uint64_t messages_received;
    uint64_t bytes_transferred;
} ipc_system_t;

#define MAX_IPC_CHANNELS 1024
#define MAX_MESSAGE_SIZE (64 * 1024)  /* 64KB */
#define MAX_SHARED_MEMORY_SIZE (16 * 1024 * 1024)  /* 16MB */

/* IPC API */
status_t ipc_init(void);
status_t ipc_shutdown(void);

/* Message Queues */
status_t ipc_create_message_queue(uint32_t max_messages, uint32_t max_message_size, uint32_t* queue_id);
status_t ipc_destroy_message_queue(uint32_t queue_id);
status_t ipc_send_message(uint32_t queue_id, uint32_t type, const void* data, size_t size, uint32_t priority);
status_t ipc_receive_message(uint32_t queue_id, uint32_t* type, void* buffer, size_t* size, bool blocking);
status_t ipc_peek_message(uint32_t queue_id, uint32_t* type, size_t* size);

/* Shared Memory */
status_t ipc_create_shared_memory(size_t size, uint32_t permissions, uint32_t* segment_id);
status_t ipc_attach_shared_memory(uint32_t segment_id, void** addr);
status_t ipc_detach_shared_memory(uint32_t segment_id);
status_t ipc_destroy_shared_memory(uint32_t segment_id);

/* Semaphores */
status_t ipc_create_semaphore(int32_t initial_value, int32_t max_value, uint32_t* semaphore_id);
status_t ipc_destroy_semaphore(uint32_t semaphore_id);
status_t ipc_semaphore_wait(uint32_t semaphore_id, bool blocking);
status_t ipc_semaphore_post(uint32_t semaphore_id);
status_t ipc_semaphore_get_value(uint32_t semaphore_id, int32_t* value);

/* Pipes */
status_t ipc_create_pipe(uint32_t* read_fd, uint32_t* write_fd);
status_t ipc_create_named_pipe(const char* name, uint32_t* pipe_id);
status_t ipc_destroy_pipe(uint32_t pipe_id);

/* High-level Channel API */
status_t ipc_create_channel(pid_t peer_pid, ipc_type_t type, uint32_t* channel_id);
status_t ipc_connect_channel(uint32_t channel_id);
status_t ipc_send_channel_data(uint32_t channel_id, const void* data, size_t size);
status_t ipc_receive_channel_data(uint32_t channel_id, void* buffer, size_t* size);
status_t ipc_close_channel(uint32_t channel_id);

/* Statistics and Monitoring */
status_t ipc_get_statistics(uint64_t* messages_sent, uint64_t* messages_received, uint64_t* bytes_transferred);
status_t ipc_get_channel_info(uint32_t channel_id, ipc_type_t* type, pid_t* creator, pid_t* peer);
status_t ipc_enum_channels(pid_t pid, uint32_t* channel_ids, uint32_t* count);