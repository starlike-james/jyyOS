#include <common.h>
#include <macro.h>

static void os_init() { pmm->init(); }
void *ptr_all[8][1024];

static void os_run() {

    printf("Hello World from CPU #%d\n", cpu_current());
    size_t align = 2;
    void **ptr = ptr_all[cpu_current()];
    /*for(int i = 0; i < 4; i++){
            ptr[i] = pmm->alloc(align);
            printf("pmm alloc %x success!, ptr = %x\n", align, ptr[i]);
            if(ptr[i] == NULL){
                break;
            }
        }
    for(int i = 0; i < 4; i++){
         if(ptr[i] == NULL){
                continue;
          }
          pmm->free(ptr[i]);
          printf("pmm free %x success!, ptr = %x\n", align, ptr[i]);
      }*/

    while (align < (4096 * KiB)) {
        for (int j = 0; j < 10; j++) {

            memset(ptr, 0, 1024 * sizeof(void *));
            int i = 0;
            for (i = 0; i < 512; i++) {
                ptr[i] = pmm->alloc(align);
                if (ptr[i] != NULL) {
                    memset(ptr[i], 100, align);
                }
                // printf("pmm alloc %x success!, ptr = %x\n", align, ptr[i]);
                align = align / 2;
                i++;
                ptr[i] = pmm->alloc(align);
                if (ptr[i] != NULL) {
                    memset(ptr[i], 100, align);
                }
                // printf("pmm alloc %x success!, ptr = %x\n", align, ptr[i]);
                align = align * 2;
                if (ptr[i] == NULL) {
                    break;
                }
            }
            size_t tem = align;
            for (; i < 1024; i++) {
                ptr[i] = pmm->alloc(align);
                if (ptr[i] != NULL) {
                    memset(ptr[i], 100, align);
                }
                // printf("pmm alloc %x success!, ptr = %x\n", align, ptr[i]);
                align++;
                if (ptr[i] == NULL) {
                    break;
                }
            }
            for (int i = 0; i < 1024; i++) {
                if (ptr[i] == NULL) {
                    continue;
                }
                pmm->free(ptr[i]);
                // printf("pmm free %x success!, ptr = %x\n", align, ptr[i]);
            }
            align = tem;
        }
        align = align << 1;
    }
    // for (const char *s = "Hello World from CPU #*\n"; *s; s++) {
    //     putch(*s == '*' ? '0' + cpu_current() : *s);
    // }
    printf("finish!\n");
    while (1)
        ;
}

MODULE_DEF(os) = {
    .init = os_init,
    .run = os_run,
};
