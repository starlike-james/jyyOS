#include <common.h>
#include <macro.h>
#include <centrallist.h>
#include <slab.h>

#define SLAB_HEADER (sizeof(block_t) + sizeof(slab_t))

cpuslablist_t cpuslablist[8];

static size_t align_up(size_t s){
    size_t align = 1;
    while(align < s){
        align = align << 1;
    }
    return align;
}

static int get_order(size_t s){
    size_t align = 1;
    int order = 0;
    while(align < s){
        align = align << 1;
        order ++;
    }
    return order;
}


static slab_t* slab_init(void *ptr, size_t size){
    assert(((uintptr_t)ptr & SLAB_MASK) == 0);
    block_t* header = (block_t *)ptr;

    assert(header->size == SLAB_PAGE);
    assert(header->magic == SLAB_MEM);
    assert(header->next == NULL);
    
    slab_t* slab = (slab_t *)((uintptr_t)ptr + sizeof(block_t));
    //slab->lk = spin_init();
    slab->size = size;
    slab->ref_count = 0;
    slab->free_count = (SLAB_PAGE - (ROUNDUP(SLAB_HEADER, size))) / size;
    slab->magic = SLAB_MEM;
    return slab;
}

static slab_t *add_new_slab(slablist_t* slablist, size_t size){
    //spin_lock(&slablist->lk);
    assert(holding(&slablist->lk));

    void *ptr = central_allocate(SLAB_PAGE, true);
    if(ptr == NULL){
        return NULL;
    }
    slab_t *slab = slab_init(ptr, size);
    slab->slablist = slablist;
    slab->next = slablist->head;
    slablist->head = slab;

    //spin_unlock(&slablist->lk);
    return slab;
}

void cpuslablist_init(){
    for(int i = 0; i <= 7; i++){
        cpuslablist[i].lk = spin_init();
        size_t size = 16;
        for(int j = 0; j <= 11; j++){
            slablist_t* slablist = &cpuslablist[i].slablist[j];
            slablist->lk = spin_init();
            slablist->size = size;
            slablist->head = NULL;
            size = size << 1;
        }
        assert(size == (64 * KiB));
    }
}

void *slab_allocate(size_t size){
    int cpu_index = cpu_current();
    size_t align = align_up(size);
    int order = get_order(size);
    if(order < 4){
        order = 4;
        align = 16;
    }
    slablist_t *slablist = &cpuslablist[cpu_index].slablist[order - 4]; 

    assert(slablist->size == align);

    spin_lock(&slablist->lk);

    slab_t *slab = slablist->head;
    
    if(slab == NULL){
        slab = add_new_slab(slablist, align);
    }
    else{
        // while(slab->free_count == 0 && slab->next != NULL){
        //     slab = slab->next;
        // }
        if(slab->free_count == 0){
            slab = add_new_slab(slablist, align);
        }
    } 

    if(slab == NULL){
        spin_unlock(&slablist->lk);
        return NULL;
    }


    //spin_lock(&slab->lk);
    assert(slab->free_count > 0);
    assert(slab->magic == SLAB_MEM);
    assert(slab->size == align);
    assert(slab->slablist == slablist);

    block_t *header = (block_t *)((uintptr_t)slab & (~SLAB_MASK));
    
    assert(header->magic == SLAB_MEM);
    assert(header->size == SLAB_PAGE);
    assert(header->next == NULL);

    uintptr_t first_ele = ROUNDUP((uintptr_t)slab + sizeof(slab_t), align);
    int total_count = (SLAB_PAGE - (ROUNDUP(SLAB_HEADER, align))) / align;
    int use_count = total_count - slab->free_count;
    void *ptr = (void *)(first_ele + use_count * align);
    slab->free_count --;
    slab->ref_count ++;
    
    spin_unlock(&slablist->lk);
    //spin_unlock(&slab->lk);
    return ptr;
} 

void slab_free(void *ptr){
    uintptr_t addr = (uintptr_t)ptr;
    assert((addr & SLAB_MASK) != 0);
    block_t *header = (block_t *)(addr & (~SLAB_MASK)); 
    if(header->magic != SLAB_MEM){
        printf("ptr = %x magic = %x\n", ptr, header->magic);
    }
    assert(header->magic == SLAB_MEM);
    assert(header->size == SLAB_PAGE);
    assert(header->next == NULL);
    
    slab_t *slab = (slab_t *)((uintptr_t)header + sizeof(block_t));
    assert(slab->magic == SLAB_MEM);

    slablist_t *slablist = slab->slablist;
    spin_lock(&slablist->lk);
    
    slab->ref_count--;
    if(slab->free_count == 0 && slab->ref_count == 0){
        slab_t *prev = slablist->head;
        if(prev == slab){
            slablist->head = NULL;
        }

        // else{
        //     while(prev->next != NULL && prev->next != slab){
        //         prev = prev->next;
        //     }
        //     assert(prev->next == slab);
        //     prev->next = slab->next;
        // }
        //central_free(header, true);
    }

    spin_unlock(&slablist->lk);
}

