#include "am.h"
#include "arch/x86_64-qemu.h"
#include <common.h>
#include <macro.h>
#include <os.h>
#include <signal.h>
#include <stdint.h>
#include <devices.h>

// #define MAX_CPU 8
// extern task_t *percpu_current[];
// extern task_t idle[];
// extern tasklist_t tasklist[];

static inline task_t *task_alloc() { return pmm->alloc(sizeof(task_t)); }

// static sem_t empty, fill;
// #define P kmt->sem_wait
// #define V kmt->sem_signal
//
// static void T_produce(void *arg) {
//     while (1) {
//         P(&empty);
//         putch('(');
//         V(&fill);
//     }
// }
// static void T_consume(void *arg) {
//     while (1) {
//         P(&fill);
//         putch(')');
//         V(&empty);
//     }
// }
//
// static void run_test1() {
//     kmt->sem_init(&empty, "empty", 4);
//     kmt->sem_init(&fill, "fill", 0);
//     for (int i = 0; i < 5; i++) {
//         kmt->create(task_alloc(), "producer", T_produce, NULL);
//     }
//     for (int i = 0; i < 5; i++) {
//         kmt->create(task_alloc(), "consumer", T_consume, NULL);
//     }
// }

// static void delay() {
//     for (int volatile i = 0;
//          i < 1000000; i++);
// }
//
// static void T1(void *arg) {
//     int i = 0;
//     while (1) {
//         putch('A');
//         iset(true);
//         if(i == 0){
//             iset(false);
//             yield();
//             i = 1;
//         }
//         delay();
//     }
// }
//
// static void T2(void *arg) { while (1) { putch('B'); delay(); } }
// static void T3(void *arg) { while (1) { putch('C'); delay(); } }
//
// static void run_test2(){
//     for(int i = 0; i < 3; i++){
//         kmt->create(task_alloc(), "A", T1, NULL);
//         kmt->create(task_alloc(), "B", T2, NULL);
//         kmt->create(task_alloc(), "C", T3, NULL);
//     }
// }

static void tty_reader(void *arg){
    device_t *tty = dev->lookup(arg);
    char cmd[128], resp[128], ps[128];
    sprintf(ps, "(%s $ ", (char *)arg);
    while(1){
        tty->ops->write(tty, 0, ps, strlen(ps));
        int nread = tty->ops->read(tty, 0, cmd, sizeof(cmd) - 1);
        cmd[nread] = '\0';
        sprintf(resp, "tty reader task: got %d character(s).\n", strlen(cmd));
        tty->ops->write(tty, 0, resp, strlen(resp));
    }
}

static void run_test3(){
    kmt->create(task_alloc(), "tty_reader", tty_reader, "tty1");
    kmt->create(task_alloc(), "tty_reader", tty_reader, "tty2");
}



static void os_init() {
    pmm->init();
    kmt->init();

#ifdef DEBUG
    // run_test1();

    // run_test2();

    dev->init();
    run_test3();
#endif
}

void *ptr_all[8][1024];

static void os_run() {
    // logging("Hello World from CPU #%d\n", cpu_current());
    iset(true);
    while (1){
        // delay();
        yield();
    }
}

handlerlist_t *handlerlist = NULL;

static bool sane_context(Context *ctx) {
    return ctx->rip >= 0x100000 && ctx->rip < (uintptr_t)heap.start;
}

static Context *os_trap(Event ev, Context *ctx) {

    Context *next = NULL;
    handlerlist_t *cur = handlerlist;
    while (cur != NULL) {
        if (cur->event == EVENT_NULL || cur->event == ev.event) {
            Context *r = cur->handler(ev, ctx);
            panic_on(r && next, "returning multiple contexts");
            if (r)
                next = r;
        }
        if (cur->next != NULL) {
            assert(cur->seq <= cur->next->seq);
        }
        cur = cur->next;
    }

    panic_on(!next, "returning NULL context");
    dpanic_on(!sane_context(next), "returning to invalid context");
    return next;
}

static void os_on_irq(int seq, int event, handler_t handler) {
    handlerlist_t *cur = pmm->alloc(sizeof(handlerlist_t));
    cur->seq = seq;
    cur->event = event;
    cur->handler = handler;
    cur->next = NULL;

    if (handlerlist == NULL || cur->seq <= handlerlist->seq) {
        cur->next = handlerlist;
        handlerlist = cur;
        return;
    }

    handlerlist_t *h = handlerlist;
    while (h->next != NULL && h->next->seq < cur->seq) {
        h = h->next;
    }

    cur->next = h->next;
    h->next = cur;
    return;
}

MODULE_DEF(os) = {
    .init = os_init,
    .run = os_run,
    .trap = os_trap,
    .on_irq = os_on_irq,
};
