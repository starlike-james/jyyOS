#include <common.h>
#include <spinlock.h>
#include <centrallist.h>
#include <macro.h>


static void *kalloc(size_t size) {
    // You can add more .c files to the repo.
    if (size > 16 * MiB){
        return NULL;
    }
    if (size > 32 * KiB){
        central_allocate(size, false);
    }
    
    return NULL;
}

static void kfree(void *ptr) {
    uintptr_t addr = (uintptr_t) ptr;
    if((addr & SLAB_MASK) == 0){
        central_free(ptr, false);
    }else{
        ;
    }
}

static void pmm_init() {
    uintptr_t pmsize = (
        (uintptr_t)heap.end
        - (uintptr_t)heap.start
    );
    
    central_init((uintptr_t)heap.start, (uintptr_t)heap.end);
    
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
