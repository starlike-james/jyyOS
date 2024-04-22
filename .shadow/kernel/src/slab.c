#include <common.h>
#include <macro.h>
#include <centrallist.h>
#include <slab.h>

cpuslablist_t cpuslablist[8];

slab_t* slab_init(void *ptr, size_t size){
    assert(((uintptr_t)ptr & SLAB_MASK) == 0);
    block_t* header = (block_t *)ptr;

    assert(header->size == SLAB_PAGE);
    assert(header->magic == SLAB_MEM);
    assert(header->next == NULL);
    
    slab_t* slab = (slab_t *)((uintptr_t)ptr + sizeof(block_t));
    slab->lk = spin_init();
    slab->size = size;
    return slab;
}

void *add_new_slab(slablist_t* slablist, size_t size){
    spin_lock(&slablist->lk);

    void *ptr = central_allocate(SLAB_PAGE, true);
    slab_t *slab = slab_init(ptr, size);
    slab->next = slablist->head;
    slablist->head = slab;

    spin_unlock(&slablist->lk);
    return ptr;
}


