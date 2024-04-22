#include <common.h>
#include <macro.h>

static void os_init() {
    pmm->init();
}

static void os_run() {

    printf("Hello World from CPU #%d\n", cpu_current());
    size_t align = 1 << 8;
        void *ptr[1024];
        memset(ptr, 0, 1024);
        int i = 0;
        for(i = 0; i < 4; i++){
            ptr[i] = pmm->alloc(align);
            printf("pmm alloc %x success!, ptr = %x\n", align, ptr[i]);
            align = align / 2;
            i++;
            ptr[i] = pmm->alloc(align);
            printf("pmm alloc %x success!, ptr = %x\n", align, ptr[i]);
            align = align * 2;
            if(ptr[i] == NULL){
                break;
            }
        }
        size_t tem = align;
        for(; i < 8; i++){
            ptr[i] = pmm->alloc(align);
            printf("pmm alloc %x success!, ptr = %x\n", align, ptr[i]);
            align++;
            if(ptr[i] == NULL){
                break;
            }
        }
        for(int i = 0; i < 8; i++){
            if(ptr[i] == NULL){
                continue;
            }
            pmm->free(ptr[i]);
            printf("pmm free %x success!, ptr = %x\n", align, ptr[i]);
        }
        align = tem;
        align = align << 1;

    while(align < (1024 * KiB)){
            }
    printf("finish!\n");
   //for (const char *s = "Hello World from CPU #*\n"; *s; s++) {
    //    putch(*s == '*' ? '0' + cpu_current() : *s);
    //}
    while (1) ;
}

MODULE_DEF(os) = {
    .init = os_init,
    .run  = os_run,
};
