#include <freelist.h>
#include <spinlock.h>

node_t *create_node(void *addr, uintptr_t size){
    node_t *node = (node_t *)addr;
    node->size = size;
    node->next = NULL;
    return node;
}

void freelist_init(list_t *list){
    list->head = NULL;
    list->lk = spin_init();
}

int freelist_insert(list_t* list, node_t *node){
    spin_lock(&list->lk);
    node->next = list->head;
    list->head = node;
    spin_unlock(&list->lk);
    return 0;
}

int freelist_del(list_t* list, node_t *node){
    return 0;
}


/*MODULE_DEF(freelist) = {
    .init = freelist_init,
};*/

