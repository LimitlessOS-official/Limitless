#include <stdlib.h>
#include <syscall.h>

// Simple implementation of malloc using sbrk
// This is a very basic allocator - production systems would use a more sophisticated approach

#define BLOCK_SIZE sizeof(struct block)

struct block {
    size_t size;
    int free;
    struct block *next;
};

static struct block *free_list = NULL;

// Find a free block that fits the requested size
static struct block *find_free_block(struct block **last, size_t size) {
    struct block *current = free_list;
    while (current && !(current->free && current->size >= size)) {
        *last = current;
        current = current->next;
    }
    return current;
}

// Request more memory from the kernel
static struct block *request_space(struct block *last, size_t size) {
    struct block *block;
    block = (struct block *)syscall1(SYS_SBRK, size + BLOCK_SIZE);
    if (block == (void *)-1) {
        return NULL; // sbrk failed
    }
    
    if (last) {
        last->next = block;
    }
    block->size = size;
    block->free = 0;
    block->next = NULL;
    return block;
}

void *malloc(size_t size) {
    if (size == 0) {
        return NULL;
    }
    
    struct block *block;
    
    if (!free_list) {
        // First call to malloc
        block = request_space(NULL, size);
        if (!block) {
            return NULL;
        }
        free_list = block;
    } else {
        struct block *last = free_list;
        block = find_free_block(&last, size);
        if (!block) {
            // No free block found, request more memory
            block = request_space(last, size);
            if (!block) {
                return NULL;
            }
        } else {
            // Found a free block
            block->free = 0;
        }
    }
    
    return (block + 1); // Return pointer to the data section (after the header)
}

void free(void *ptr) {
    if (!ptr) {
        return;
    }
    
    struct block *block = (struct block *)ptr - 1;
    block->free = 1;
    
    // TODO: Coalesce adjacent free blocks for better memory utilization
}

void *calloc(size_t num, size_t size) {
    size_t total_size = num * size;
    void *ptr = malloc(total_size);
    if (ptr) {
        // Zero out the memory
        char *p = (char *)ptr;
        for (size_t i = 0; i < total_size; i++) {
            p[i] = 0;
        }
    }
    return ptr;
}

void *realloc(void *ptr, size_t size) {
    if (!ptr) {
        return malloc(size);
    }
    
    if (size == 0) {
        free(ptr);
        return NULL;
    }
    
    struct block *block = (struct block *)ptr - 1;
    if (block->size >= size) {
        // Current block is large enough
        return ptr;
    }
    
    // Need to allocate a new block
    void *new_ptr = malloc(size);
    if (new_ptr) {
        // Copy old data
        char *src = (char *)ptr;
        char *dst = (char *)new_ptr;
        for (size_t i = 0; i < block->size && i < size; i++) {
            dst[i] = src[i];
        }
        free(ptr);
    }
    return new_ptr;
}
