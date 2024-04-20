#ifndef FREELIST_H__
#define FREELIST_H__

#include <spinlock.h>


typedef struct __node_t{
    uintptr_t size;
    struct __node_t* next; 
}node_t;

typedef struct {
    int size;
    int magic;
}header_t;

typedef struct {
    node_t *head;
    spinlock_t lk;
}list_t;

node_t *create_node(void *addr, uintptr_t size);

void freelist_init(list_t *list);
int freelist_insert(list_t *list, node_t *node);
int freelist_del(list_t *list, node_t *node);

/*MODULE(freelist){
    list_t list;
    void (*init)(list_t *);
    void (*insert)(node_t *);
    void (*del)(node_t *);
};*/
#endif
