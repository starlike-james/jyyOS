#include "co.h"
#include <stdlib.h>
#include <ucontext.h>
#include <assert.h>
#include <time.h>
#include <stdint.h>

#define STACK_SIZE 65536 

enum{
    WORKING = 0,
    FINISH,
};

struct co {
    ucontext_t ucontext;
    void (*func)(void *);
    void *arg;
    int state;
};

//struct co* head;
static struct co* current = NULL;
//struct co* co_main = NULL;
static struct co* colist[128];
static int conum = 0;

static void co_func_wrapper(struct co *co){
    co->func(co->arg);
    co->state = FINISH;
    co_yield();
}

#ifdef __x86_64__
static void co_func_outter_wrapper(int high, int low) {
  struct co *co = (struct co*)(((uintptr_t)high << 32) | low);
  co_func_wrapper(co);
}
static void makecontext_wrap(struct co *co) {
    uint32_t high = (uint32_t)(((uintptr_t) co >> 32) & 0xffffffff);
    uint32_t low  = (uint32_t)(((uintptr_t) co & 0xffffffff));
    makecontext(&co->ucontext, (void (*)())co_func_outter_wrapper, 2, high, low);
}

#else

static void co_func_outter_wrapper(int addr) {
    struct co *co = (struct co *)(uintptr_t)addr;
    co_func_wrapper(co);
}
static void makecontext_wrap(struct co *co) {
    makecontext(&co->ucontext, (void (*)())co_func_outter_wrapper, 1, (uintptr_t)co);
}
#endif

struct co *co_start(const char *name, void (*func)(void *), void *arg) {
    int idx = 1; // colist[0] is main
    while(colist[idx] != NULL){
        idx++;
    }
    colist[idx] = malloc(sizeof(struct co));
    assert(colist[idx]);
    struct co* co = colist[idx];
    conum++;
    getcontext(&(co->ucontext));
    co->ucontext.uc_link = NULL;
    co->ucontext.uc_stack.ss_sp = malloc(STACK_SIZE);
    assert(co->ucontext.uc_stack.ss_sp);
    co->ucontext.uc_stack.ss_size = STACK_SIZE;
    co->func = func;
    co->arg = arg;
    co->state = WORKING;
    makecontext_wrap(co);

    return co;
}

void co_wait(struct co *co) {
    while(co->state != FINISH){
        co_yield();
    }
    conum--;
    free(co->ucontext.uc_stack.ss_sp);
    free(co);
    co = NULL;
}

void co_yield() {
    if(current == NULL){
        srand(time(NULL));
        struct co *co_main = colist[0];
        co_main = malloc(sizeof(struct co));
        assert(co_main);
        conum++;
        //getcontext(&co_main->ucontext);
        co_main->func = NULL;
        co_main->arg = NULL;
        co_main->state = WORKING;
        current = co_main;
    }
    struct co* prev = current;
    int random = rand() % conum;
    int idx = -1;
    int cnt = 0;
    while(cnt != random){
        idx++;
        if(colist[idx] != NULL){
            cnt++;
        }
    }
    assert(colist[idx]);
    current = colist[idx];
    swapcontext(&prev->ucontext, &current->ucontext);
}
