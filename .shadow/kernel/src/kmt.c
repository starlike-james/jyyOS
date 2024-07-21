#include "am.h"
#include <common.h>
#include <limits.h>
#include <os.h>
#include <spinlock.h>
#include <stdint.h>

#define STACK_SIZE 8192

#define MAX_CPU 8

task_t *percpu_current[MAX_CPU];
tasklist_t tasklist;
task_t idle[MAX_CPU];

static Context *kmt_save_context(Event ev, Context *ctx);
static Context *kmt_schedule(Event ev, Context *ctx);

static void kmt_init() {

    kmt->spin_init(&tasklist.lk, "tasklist");
    tasklist.head = NULL;

    for (int i = 0; i < MAX_CPU; i++) {
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

    tasklist_t *curlist = &tasklist;

    kmt->spin_lock(&curlist->lk);
    task_t *nexttask = NULL;
    if (curtask == NULL) {
        logging("cpu%d: schedule from idle\n", cpu_current());
        nexttask = curlist->head;
        if (nexttask != NULL) {
            while (nexttask->next != NULL) {
                if (nexttask->status == READY) {
                    break;
                }
                nexttask = nexttask->next;
            }
        }
    } else {
        logging("cpu%d: schedule from %s\n", cpu_current(), curtask->name);
        nexttask = curtask->next;
        if (nexttask == NULL) {
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
        kmt->spin_lock(&curtask->lk);

        if (curtask->status == RUNNING) {
            curtask->status = READY;
        } else if (curtask->status == TOBLOCK) {
            curtask->status = BLOCKED;
        } else if (curtask->status == BLOCKED){
            curtask->status = READY;
        } else{
            assert(curtask == NULL);
        }

        kmt->spin_unlock(&curtask->lk);
        curtask = NULL;
        ret = &idle[cpu_current()].context;
        logging("cpu%d : schedule to idle\n", cpu_current());
    } else {
        kmt->spin_lock(&curtask->lk);

        if (curtask->status == RUNNING) {
            curtask->status = READY;
        } else if (curtask->status == TOBLOCK) {
            curtask->status = BLOCKED;
        } else if (curtask->status == BLOCKED){
            curtask->status = READY;
        } else{
            assert(curtask == NULL);
        }

        kmt->spin_unlock(&curtask->lk);

        curtask = nexttask;

        kmt->spin_lock(&curtask->lk);

        assert(nexttask->status == READY);
        curtask->status = RUNNING;

        kmt->spin_unlock(&curtask->lk);
        ret = &curtask->context;
        logging("cpu%d: schedule to %s\n", cpu_current(), curtask->name);
    }
    kmt->spin_unlock(&curlist->lk);
    return ret;
#undef curtask
}

static void add_tasklist(task_t *task) {
    tasklist_t *curlist = &tasklist;
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

    kmt->spin_lock(&tasklist.lk);
    if (tasklist.head == task) {
        tasklist.head = task->next;
    } else {
        task_t *curtask = tasklist.head;
        while (curtask->next != task) {
            curtask = curtask->next;
            if (curtask == NULL) {
                break;
            }
        }
        assert(curtask->next == task);
        curtask->next = task->next;
    }

    kmt->spin_unlock(&tasklist.lk);

    task->next = NULL;
    task->cpuid = -1;
}

static void spin_init(spinlock_t *lk, const char *name) {
    lk->name = name;
    lk->lk = lspin_init();
}

static void spin_lock(spinlock_t *lk) { lspin_lock(&lk->lk); }

static void spin_unlock(spinlock_t *lk) { lspin_unlock(&lk->lk); }

static void sem_init(sem_t *sem, const char *name, int value) {
    kmt->spin_init(&sem->spinlock, name);
    sem->name = name;
    sem->count = value;
    kmt->spin_init(&sem->waitlist.lk, "waitlist");
    sem->waitlist.head = NULL;
}

static void sem_wait(sem_t *sem) {
    int acquired = 0;
    kmt->spin_lock(&sem->spinlock);
    if (sem->count == 0) {
        waitlistnode_t *curtask = pmm->alloc(sizeof(waitlistnode_t));
        curtask->task = percpu_current[cpu_current()];
        curtask->next = NULL;

        kmt->spin_lock(&sem->waitlist.lk);
        if (sem->waitlist.head == NULL) {
            sem->waitlist.head = curtask;
        } else {
            waitlistnode_t *waitlist_end = sem->waitlist.head;
            while (waitlist_end->next != NULL) {
                waitlist_end = waitlist_end->next;
            }
            assert(waitlist_end->next == NULL);
            waitlist_end->next = curtask;
        }
        kmt->spin_lock(&curtask->task->lk);
        curtask->task->status = TOBLOCK;
        kmt->spin_unlock(&curtask->task->lk);
        kmt->spin_unlock(&sem->waitlist.lk);
    } else {
        sem->count--;
        assert(sem->count >= 0);
        acquired = 1;
    }
    kmt->spin_unlock(&sem->spinlock);
    if (!acquired) {
        yield();
    }
}

static void sem_signal(sem_t *sem) {
    kmt->spin_lock(&sem->spinlock);
    kmt->spin_lock(&sem->waitlist.lk);
    if (sem->waitlist.head != NULL) {
        task_t *task = sem->waitlist.head->task;
        kmt->spin_lock(&task->lk);
        if (task->status == TOBLOCK) {
            task->status = BLOCKED;
        } else if (task->status == BLOCKED) {
            task->status = READY;
        } else {
            assert(0);
        }
        kmt->spin_unlock(&task->lk);
        sem->waitlist.head = sem->waitlist.head->next;
    } else {
        sem->count++;
    }
    kmt->spin_unlock(&sem->waitlist.lk);
    kmt->spin_unlock(&sem->spinlock);
}

MODULE_DEF(kmt) = {
    .init = kmt_init,
    .create = task_create,
    .teardown = task_teardown,
    .spin_init = spin_init,
    .spin_lock = spin_lock,
    .spin_unlock = spin_unlock,
    .sem_init = sem_init,
    .sem_wait = sem_wait,
    .sem_signal = sem_signal,
};
