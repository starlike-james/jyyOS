#include "co.h"
#include <stdlib.h>
#include <ucontext.h>
#include <assert.h>
#include <time.h>

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
    makecontext(&(co->ucontext), (void (*)(void))co_func_wrapper, 1, co);

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
    int idx = 0;
    int cnt = 0;
    while(cnt != random){
        if(colist[idx] != NULL){
            cnt++;
        }
        idx++;
    }
    current = colist[idx];
    swapcontext(&prev->ucontext, &current->ucontext);
}
