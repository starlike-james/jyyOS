#include <common.h>
#include <macro.h>

static void os_init() {
    pmm->init();
}

static void os_run() {

    printf("Hello World from CPU #%d\n", cpu_current());
    size_t align = (1 << 13) + 0x3ff;
    while(align < (1024 * KiB)){
        for(int i = 0; i < 1024; i++){
            void *ptr = pmm->alloc(align);
            printf("pmm alloc %x success!, ptr = %x\n", align, ptr);
            pmm->free(ptr);
            printf("pmm free %x success!, ptr = %x\n", align, ptr);
        }
        size_t tem = align;
        for(int i = 0; i < 1024; i++){
            void *ptr = pmm->alloc(align);
            printf("pmm alloc %x success!, ptr = %x\n", align, ptr);
            pmm->free(ptr);
            printf("pmm free %x success!, ptr = %x\n", align, ptr);
            align++;
        }
        align = tem;
        align = align << 1;
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
