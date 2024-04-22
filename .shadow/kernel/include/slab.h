#ifndef SLAB_H__
#define SLAB_H__

#include <spinlock.h>

//typedef struct __slablist_t slablist_t;

typedef struct __slab_t{
    size_t size;
    int free_count;
    int ref_count;
    struct __slab_t *next;
    struct __slablist_t* slablist;
    // spinlock_t lk;
    uint32_t magic;
}slab_t;

typedef struct __slablist_t{
    size_t size;
    slab_t* head;
    spinlock_t lk;
}slablist_t;

typedef struct __cpuslablist_t{
    slablist_t slablist[12];
    spinlock_t lk;
}cpuslablist_t;

#endif
