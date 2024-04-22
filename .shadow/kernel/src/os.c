#include <common.h>

static void os_init() {
    pmm->init();
}

static void os_run() {

    printf("Hello World from CPU #%d\n", cpu_current());
    void *ptr = pmm->alloc(2);
    printf("pmm alloc success!\n");
    pmm->free(ptr);
    printf("pmm free success!\n");
    //for (const char *s = "Hello World from CPU #*\n"; *s; s++) {
    //    putch(*s == '*' ? '0' + cpu_current() : *s);
    //}
    while (1) ;
}

MODULE_DEF(os) = {
    .init = os_init,
    .run  = os_run,
};
