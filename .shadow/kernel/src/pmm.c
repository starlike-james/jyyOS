#include <common.h>
#include <spinlock.h>

#define MiB * (1024LL * 1024)

static size_t align_up(size_t s){
    size_t align = 0;
    while(align < s){
        align = align << 1;
    }
    return align;
}

static void *kalloc(size_t size) {
    // You can add more .c files to the repo.
    if (size > 16 MiB){
        return NULL;
    }
    size_t align = align_up(size);
    align = align + 1;
    return NULL;
}

static void kfree(void *ptr) {
    // TODO
    // You can add more .c files to the repo.
}

static void pmm_init() {
    uintptr_t pmsize = (
        (uintptr_t)heap.end
        - (uintptr_t)heap.start
    );

    printf(
        "Got %d MiB heap: [%p, %p)\n",
        pmsize >> 20, heap.start, heap.end
    );
}

MODULE_DEF(pmm) = {
    .init  = pmm_init,
    .alloc = kalloc,
    .free  = kfree,
};
