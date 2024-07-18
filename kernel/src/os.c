#include "am.h"
#include "arch/x86_64-qemu.h"
#include <common.h>
#include <os.h>
#include <macro.h>
#include <signal.h>
#include <stdint.h>

static void os_init() { 
    pmm->init();
    kmt->init();
}

void *ptr_all[8][1024];

static void os_run() {
    // printf("Hello World from CPU #%d\n", cpu_current());
    // size_t align = 2;
    // void **ptr = ptr_all[cpu_current()];
    // while (align < (4096 * KiB)) {
    //     for (int j = 0; j < 10; j++) {
    //
    //         memset(ptr, 0, 1024 * sizeof(void *));
    //         int i = 0;
    //         for (i = 0; i < 512; i++) {
    //             ptr[i] = pmm->alloc(align);
    //             if (ptr[i] != NULL) {
    //                 memset(ptr[i], 100, align);
    //             }
    //             // printf("pmm alloc %x success!, ptr = %x\n", align, ptr[i]);
    //             align = align / 2;
    //             i++;
    //             ptr[i] = pmm->alloc(align);
    //             if (ptr[i] != NULL) {
    //                 memset(ptr[i], 100, align);
    //             }
    //             // printf("pmm alloc %x success!, ptr = %x\n", align, ptr[i]);
    //             align = align * 2;
    //             if (ptr[i] == NULL) {
    //                 break;
    //             }
    //         }
    //         size_t tem = align;
    //         for (; i < 1024; i++) {
    //             ptr[i] = pmm->alloc(align);
    //             if (ptr[i] != NULL) {
    //                 memset(ptr[i], 100, align);
    //             }
    //             // printf("pmm alloc %x success!, ptr = %x\n", align, ptr[i]);
    //             align++;
    //             if (ptr[i] == NULL) {
    //                 break;
    //             }
    //         }
    //         for (int i = 0; i < 1024; i++) {
    //             if (ptr[i] == NULL) {
    //                 continue;
    //             }
    //             pmm->free(ptr[i]);
    //             // printf("pmm free %x success!, ptr = %x\n", align, ptr[i]);
    //         }
    //         align = tem;
    //     }
    //     align = align << 1;
    // }
    // printf("finish!\n");
    while (1);
}


handlerlist_t *handlerlist = NULL;

static bool sane_context(Context *ctx){
    return ctx->rip >= 0x100000 && ctx->rip < (uintptr_t)heap.start;
}

static Context *os_trap(Event ev, Context *ctx) {

    Context *next = NULL;
    handlerlist_t *current = handlerlist;
    while (current != NULL) {
        if (current->event == EVENT_NULL || current->event == ev.event) {
            Context *r = current->handler(ev, ctx);
            panic_on(r && next, "returning multiple contexts");
            if (r) next = r;
        }
        if(current->next != NULL){
            assert(current->seq <= current->next->seq);
        }
        current = current->next;
    }
    panic_on(!next, "returning NULL context");
    panic_on(!sane_context(next), "returning to invalid context");
    return next;
}

static void os_on_irq(int seq, int event, handler_t handler){
    handlerlist_t *current = pmm->alloc(sizeof(handlerlist_t));
    current->seq = seq;
    current->event = event;
    current->handler = handler;
    current->next = NULL;

    if(handlerlist == NULL || current->seq <= handlerlist->seq){
        current->next = handlerlist;
        handlerlist = current;
        return;
    }

    handlerlist_t *h = handlerlist;
    while(h->next != NULL && h->next->seq < current->seq){
        h = h->next;
    }

    current->next = h->next;
    h->next = current; 
    return;
}

MODULE_DEF(os) = {
    .init = os_init,
    .run = os_run,
    .trap = os_trap,
    .on_irq = os_on_irq,
};
