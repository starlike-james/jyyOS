#include <am.h>
#include <common.h>
#include <os.h>
#include <spinlock.h>

// This is a ported version of spin-lock
// from xv6-riscv to AbstractMachine:
// https://github.com/mit-pdos/xv6-riscv

struct lcpu lcpus[16];

void push_off();
void pop_off();
bool holding(lspinlock_t *lk);

void lspin_lock(lspinlock_t *lk) {
    // Disable interrupts to avoid deadlock.
    logging("before push off\n");
    push_off();

    // This is a deadlock.
    logging("spinlock\n");
    if (holding(lk)) {
        panic("have acquired the same lock before!");
    }

    // This our main body of spin lock.
    int got;
    do {
        got = atomic_xchg(&lk->status, LOCKED);
    } while (got != UNLOCKED);

    lk->lcpu = mycpu;
}

void lspin_unlock(lspinlock_t *lk) {
    logging("spinunlock\n");
    if (!holding(lk)) {
        panic("have released the same lock before!");
    }

    lk->lcpu = NULL;
    atomic_xchg(&lk->status, UNLOCKED);

    pop_off();
}

// Check whether this cpu is holding the lock.
// Interrupts must be off.
bool holding(lspinlock_t *lk) {
    return (lk->status == LOCKED && lk->lcpu == &lcpus[cpu_current()]);
}

// push_off/pop_off are like intr_off()/intr_on()
// except that they are matched:
// it takes two pop_off()s to undo two push_off()s.
// Also, if interrupts are initially off, then
// push_off, pop_off leaves them off.
void push_off(void) {
    int old = ienabled();
    struct lcpu *c = mycpu;

    iset(false);
    if (c->noff == 0) {
        c->intena = old;
    }
    c->noff += 1;
}

void pop_off(void) {
    struct lcpu *c = mycpu;

    // Never enable interrupt when holding a lock.
    if (ienabled()) {
        panic("pop_off - interruptible");
    }

    if (c->noff < 1) {
        panic("pop_off");
    }

    c->noff -= 1;
    if (c->noff == 0 && c->intena) {
        iset(true);
    }
}

