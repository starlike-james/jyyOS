// When we test your kernel implementation, the framework
// directory will be replaced. Any modifications you make
// to its files (e.g., kernel.h) will be lost. 

// Note that some code requires data structure definitions 
// (such as `sem_t`) to compile, and these definitions are
// not present in kernel.h. 

// Include these definitions in os.h.
#ifndef OS_H__
#define OS_H__

#include <common.h>
#include <spinlock.h>

typedef struct handlerlist_t{
    handler_t handler;
    int seq;
    int event;
    struct handlerlist_t* next;
}handlerlist_t;


struct spinlock{
    lspinlock_t lk;
    const char *name;
};

struct task{
    void (*entry)(void *);
    void *arg;
    const char* name;
    spinlock_t lk;
    int status;
    int cpuid;
    Context context;
    struct task *next;
    uint8_t *stack;
};


typedef struct tasklist_t{
    spinlock_t lk;
    task_t *head;
}tasklist_t;

typedef struct waitlistnode_t{
    task_t *task;
    struct waitlistnode_t *next; 
}waitlistnode_t;

typedef struct waitlist_t{
    spinlock_t lk;
    waitlistnode_t *head;
}waitlist_t;

struct semaphore{
    spinlock_t spinlock;
    waitlist_t waitlist;
    int count;
    const char* name;
};

enum{
    UNUSED = 0,
    NEW,
    READY,
    TOREADY,
    RUNNING,
    BLOCKED,
    TOBLOCK
};

#endif
