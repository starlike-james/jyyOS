#include "am.h"
#include <centrallist.h>
#include <common.h>
#include <macro.h>
#include <slab.h>
#include <spinlock.h>
#include <stdbool.h>

static void *kalloc(size_t size) {
    // You can add more .c files to the repo.
    if (size > 16 * MiB) {
        return NULL;
    }
    void *ptr;
    if (size > 32 * KiB) {
        ptr = central_allocate(size, false);
    } else {
        ptr = slab_allocate(size);
    }
    if(ptr != NULL){
        memset(ptr, 0, size);
    }

    return ptr;
}

static void kfree(void *ptr) {
    uintptr_t addr = (uintptr_t)ptr;
    if ((addr & SLAB_MASK) == 0) {
        central_free(ptr, false);
    } else {
        slab_free(ptr);
    }
}

static void *kalloc_safe(size_t size){
    bool i = ienabled();
    iset(false);
    void *ret = kalloc(size);
    if (i) {
        iset(true);
    }
    return ret;
}

static void kfree_safe(void *ptr){
    bool i = ienabled();
    iset(false);
    kfree(ptr);
    if (i){
        iset(true);
    }
}

static void pmm_init() {
    uintptr_t pmsize = ((uintptr_t)heap.end - (uintptr_t)heap.start);

    central_init((uintptr_t)heap.start, (uintptr_t)heap.end);
    cpuslablist_init();

    printf("Got %d MiB heap: [%p, %p)\n", pmsize >> 20, heap.start, heap.end);
}

MODULE_DEF(pmm) = {
    .init = pmm_init,
    .alloc = kalloc_safe,
    .free = kfree_safe,
};
