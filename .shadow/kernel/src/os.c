#include <common.h>
#include <macro.h>

static void os_init() {
    pmm->init();
}

static void os_run() {

    printf("Hello World from CPU #%d\n", cpu_current());
    size_t align = 1;
    while(align < (1024 * KiB)){
        void *ptr = pmm->alloc(align);
        printf("pmm alloc %u success!\n", align);
        pmm->free(ptr);
        printf("pmm free %u success!\n", align); 
    }
   //for (const char *s = "Hello World from CPU #*\n"; *s; s++) {
    //    putch(*s == '*' ? '0' + cpu_current() : *s);
    //}
    while (1) ;
}

MODULE_DEF(os) = {
    .init = os_init,
    .run  = os_run,
};
