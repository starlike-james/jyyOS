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
    const char *name;
    int status;
    struct lcpu *lcpu;
} spinlock_t;

#define spin_init(name_) \
    ((spinlock_t) { \
        .name = name_, \
        .status = UNLOCKED, \
        .cpu = NULL, \
    })

void spin_lock(spinlock_t *lk);
void spin_unlock(spinlock_t *lk);

#define panic(...) \
    do { \
        printf("Panic: " __VA_ARGS__); \
        halt(1); \
    } while (0)

