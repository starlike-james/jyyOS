#include "am.h"
#include <common.h>
#include <limits.h>
#include <os.h>
#include <spinlock.h>
#include <stdint.h>

#define STACK_SIZE 8192

#define MAX_CPU 8

task_t *percpu_current[MAX_CPU];
tasklist_t tasklist[MAX_CPU];
task_t idle[MAX_CPU];

static Context *kmt_save_context(Event ev, Context *ctx);
static Context *kmt_schedule(Event ev, Context *ctx);

static void kmt_init() {
    for (int i = 0; i < MAX_CPU; i++) {
        kmt->spin_init(&tasklist[i].lk, "tasklist");
        tasklist[i].head = NULL;

        idle[i].cpuid = i;
        kmt->spin_init(&idle[i].lk, "idle");
        idle[i].name = "idle";

        percpu_current[i] = NULL;
    }
    os->on_irq(INT_MIN, EVENT_NULL, kmt_save_context);
    os->on_irq(INT_MAX, EVENT_NULL, kmt_schedule);
}

static Context *kmt_save_context(Event ev, Context *ctx) {
    task_t *curtask = percpu_current[cpu_current()];
    if (curtask == NULL) {
        idle[cpu_current()].context = *ctx;
    } else {
        curtask->context = *ctx;
    }
    return NULL;
}

static Context *kmt_schedule(Event ev, Context *ctx) {
#define curtask percpu_current[cpu_current()]
    tasklist_t *curlist = &tasklist[cpu_current()];

    kmt->spin_lock(&curlist->lk);
    task_t *nexttask = NULL;
    if (curtask == NULL) {
        logging("cpu%d: schedule from idle\n", cpu_current());
        nexttask = curlist->head;
        while (nexttask->next != NULL) {
            if (nexttask->status == READY) {
                break;
            }
            nexttask = nexttask->next;
        }
    } else {
        assert(curtask->status == RUNNING);
        logging("cpu%d: schedule from %s\n", cpu_current(), curtask->name);
        nexttask = curtask->next;
        if(nexttask == NULL){
            nexttask = curlist->head;
        }
        while (nexttask != curtask) {
            if (nexttask->status == READY) {
                break;
            }
            if (nexttask->next == NULL) {
                nexttask = curlist->head;
            } else {
                nexttask = nexttask->next;
            }
        }
        if (nexttask == curtask) {
            nexttask = NULL;
        }
    }

    Context *ret = NULL;

    if (nexttask == NULL) {
        logging("have no READY task\n");
        curtask->status = READY;
        curtask = NULL;
        ret = &idle[cpu_current()].context;
        logging("cpu%d : schedule to idle\n", cpu_current());
    } else {
        curtask->status = READY;
        curtask = nexttask;
        assert(nexttask->status == READY);
        curtask->status = RUNNING;
        ret = &curtask->context;
        logging("cpu%d: schedule to %s\n", cpu_current(), curtask->name);
    }
    kmt->spin_unlock(&curlist->lk);
    return ret;
#undef curtask
}

static void add_tasklist(task_t *task) {
    tasklist_t *curlist = &tasklist[cpu_current()];
    kmt->spin_lock(&curlist->lk);
    if (!(holding(&task->lk.lk))) {
        panic("haven't get the task's lock");
    }
    if (curlist->head == NULL) {
        curlist->head = task;
    } else {
        task_t *curtask = curlist->head;
        while (curtask->next != NULL) {
            curtask = curtask->next;
        }
        assert(curtask != NULL && curtask->next == NULL);
        curtask->next = task;
    }
    kmt->spin_unlock(&curlist->lk);
}

static int task_create(task_t *task, const char *name, void (*entry)(void *arg),
                       void *arg) {
    kmt->spin_init(&task->lk, name);
    kmt->spin_lock(&task->lk);

    task->status = NEW;
    task->stack = pmm->alloc(STACK_SIZE);
    if (task->stack == NULL) {
        logging("pmm fails to alloc stack");
        return -1;
    }
    task->name = name;
    task->entry = entry;
    task->arg = arg;
    task->cpuid = cpu_current();
    task->context =
        *kcontext((Area){.start = task->stack, task->stack + STACK_SIZE},
                  task->entry, task->arg);
    dassert(task->context.rsp == (uintptr_t)task->stack + STACK_SIZE);
    task->next = NULL;
    add_tasklist(task);
    task->status = READY;
    kmt->spin_unlock(&task->lk);

    return 0;
}

static void task_teardown(task_t *task) {
    dassert(task != NULL);

    assert(task->cpuid >= 0 && task->cpuid < MAX_CPU);

    // TODO

    pmm->free(task->stack);
    task->status = UNUSED;
    task->name = NULL;
    task->entry = NULL;
    task->arg = NULL;
    memset(&task->context, 0, sizeof(task->context));

    kmt->spin_lock(&tasklist[task->cpuid].lk);
    if (tasklist[task->cpuid].head == task) {
        tasklist[task->cpuid].head = task->next;
    } else {
        task_t *curtask = tasklist[task->cpuid].head;
        while (curtask->next != task) {
            curtask = curtask->next;
            if (curtask == NULL) {
                break;
            }
        }
        assert(curtask->next == task);
        curtask->next = task->next;
    }

    kmt->spin_unlock(&tasklist[task->cpuid].lk);

    task->next = NULL;
    task->cpuid = -1;
}

static void spin_init(spinlock_t *lk, const char *name) {
    lk->name = name;
    lk->lk = lspin_init();
}

static void spin_lock(spinlock_t *lk) { lspin_lock(&lk->lk); }

static void spin_unlock(spinlock_t *lk) { lspin_unlock(&lk->lk); }

MODULE_DEF(kmt) = {
    .init = kmt_init,
    .create = task_create,
    .teardown = task_teardown,
    .spin_init = spin_init,
    .spin_lock = spin_lock,
    .spin_unlock = spin_unlock,
};
