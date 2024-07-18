// When we test your kernel implementation, the framework
// directory will be replaced. Any modifications you make
// to its files (e.g., kernel.h) will be lost. 

// Note that some code requires data structure definitions 
// (such as `sem_t`) to compile, and these definitions are
// not present in kernel.h. 

// Include these definitions in os.h.
#include <common.h>

typedef struct handlerlist_t{
    handler_t handler;
    int seq;
    int event;
    struct handlerlist_t* next;
}handlerlist_t;

