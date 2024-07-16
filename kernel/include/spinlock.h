#ifndef SPINLOCK_H__
#define SPINLOCK_H__

#include <am.h>
#include <klib.h>

#define UNLOCKED  0
#define LOCKED    1

struct lcpu {
    int noff;
    int intena;
};

extern struct lcpu lcpus[];
#define mycpu (&lcpus[cpu_current()])

typedef struct {
    //const char *name;
    struct lcpu *lcpu;
    int status;
} lspinlock_t;

#define spin_init() \
    ((lspinlock_t) { \
        .status = UNLOCKED, \
        .lcpu = NULL, \
    })

void spin_lock(lspinlock_t *lk);
void spin_unlock(lspinlock_t *lk);
bool holding(lspinlock_t *lk);

#endif
