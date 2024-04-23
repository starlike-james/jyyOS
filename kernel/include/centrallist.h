#ifndef CENTRAL_H__
#define CENTRAL_H__

#include<spinlock.h>

typedef struct __block_t{
    size_t size;
    struct __block_t* next; 
    uint32_t magic;
}block_t;


typedef struct {
    block_t *head;
    spinlock_t lk;
}central_list_t;


void central_init(uintptr_t heap_start, uintptr_t heap_end);
void* central_allocate(size_t size, bool slab);
void central_free(void *ptr, bool slab);

#endif

