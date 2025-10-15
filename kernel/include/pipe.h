#pragma once
#include "kernel.h"

#define PIPE_BUF_SIZE 4096

typedef struct pipe {
    u8   buf[PIPE_BUF_SIZE];
    u32  read_pos;
    u32  write_pos;
    u32  data_size;
    u32  refcnt;
    spinlock_t lock;
    int  closed_read;
    int  closed_write;
} pipe_t;

int pipe_create(pipe_t** out_read, pipe_t** out_write);
int pipe_read(pipe_t* p, void* buf, u32 len);
int pipe_write(pipe_t* p, const void* buf, u32 len);
int pipe_close_read(pipe_t* p);
int pipe_close_write(pipe_t* p);
