#ifndef SLAB_H__
#define SLAB_H__

#include <spinlock.h>

typedef struct __slab_t{
    size_t size;
    struct __slab_t *next;
    spinlock_t lk;
}slab_t;

typedef struct __slablist_t{
    size_t size;
    slab_t* head;
    spinlock_t lk;
}slablist_t;

typedef struct __cpuslablist_t{
    slablist_t *slablist[12];
    spinlock_t lk;
}cpuslablist_t;

#endif
