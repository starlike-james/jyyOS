#include <common.h>
#include <centrallist.h>
#include <macro.h>

central_list_t central;

static size_t align_up(size_t s){
    size_t align = 1;
    while(align < s){
        align = align << 1;
    }
    return align;
}

void central_init(uintptr_t heap_start, uintptr_t heap_end){
    block_t* head = (block_t *)(ROUNDUP((uintptr_t)heap_start, SLAB_PAGE)); //64 KiB align
    head->size = (uintptr_t)heap.end - (uintptr_t)head;
    head->next = NULL; 
    head->magic = FREE_MEM;
    central.lk = spin_init();
    central.head = head;
}


void *central_allocate(size_t size, bool slab){
    spin_lock(&central.lk);

    block_t *cur = central.head;
    block_t *prev = NULL; 

    size_t align = align_up(size);
    size_t alloc_size = 0;
    if(slab){
        alloc_size = SLAB_PAGE;
    }
    else{
        alloc_size = ROUNDUP(size, SLAB_PAGE);
    }

    for(; cur != NULL; prev = cur, cur = cur->next){
        //assert((uintptr_t)(cur) % SLAB_PAGE == 0);
        if(slab){
            if(cur->size < SLAB_PAGE){
                continue;
            }
            assert(size == SLAB_PAGE);
            assert(((uintptr_t)cur & SLAB_MASK) == 0);

            uintptr_t ptr = (uintptr_t)cur;
            block_t* header = cur;

            if(cur->size != SLAB_PAGE){
               block_t* new = (block_t*)((uintptr_t)cur + SLAB_PAGE);
               new->size = cur->size - SLAB_PAGE;
               new->next = cur->next;
               cur->next = new;
               assert(new->magic != SLAB_MEM && new->magic != BIG_MEM);
                
               new->magic = FREE_MEM;
            }

            if(prev == NULL){
               central.head = cur->next;
            }
            else{
               prev->next = cur->next;
            }

            header->size = SLAB_PAGE;
            header->next = NULL;
            header->magic = SLAB_MEM;

            spin_unlock(&central.lk);
            return (void *)ptr;
        }
        else{
            // big memory allocate

            if(cur->size < alloc_size + SLAB_PAGE){
                continue;
            }

            uintptr_t ptr = ROUNDUP((uintptr_t)cur + SLAB_PAGE, align);
            //printf("alloc: ptr = %x cur = %x align = %x\n", (uintptr_t)ptr, (uintptr_t)cur, align);
            //uintptr_t alloc_size = ROUNDUP(size, SLAB_PAGE); 
            if((uintptr_t)cur + cur->size < ptr + alloc_size){
                continue;
            }
            
            assert((ptr & (align - 1)) == 0);
            assert(alloc_size >= SLAB_PAGE);
            assert(((uintptr_t)cur & SLAB_MASK) == 0);
            assert(ptr - SLAB_PAGE >= (uintptr_t)cur);

            block_t* header = (block_t *)(ptr - SLAB_PAGE);

            size_t new_block_size = (uintptr_t)cur + cur->size - (ptr + alloc_size);
            if(new_block_size != 0){   //can not change to > 0 since it is unsigned ll
                block_t* new = (block_t *)(ptr + alloc_size);
                new->size = new_block_size;
                new->next = cur->next;
                cur->next = new;
                if(new->magic == SLAB_MEM || new->magic == BIG_MEM){
                    printf("magic = %x\n", new->magic);
                    assert(0);
                }
                assert(new->magic != SLAB_MEM && new->magic != BIG_MEM);
                new->magic = FREE_MEM;
            } 

            size_t cur_block_size = ptr - SLAB_PAGE - (uintptr_t)cur; 
            if(cur_block_size == 0){
                if(prev == NULL){
                    central.head = cur->next;
                }
                else{
                    prev->next = cur->next;
                }
            }
            else{
                cur->size = cur_block_size;
            }

            header->size = alloc_size;
            header->next = NULL;
            header->magic = BIG_MEM;

            spin_unlock(&central.lk);
            return (void *)ptr;
        }
    }

    spin_unlock(&central.lk);
    return NULL;
}

void central_free(void *ptr, bool slab){
    spin_lock(&central.lk);

    uintptr_t addr = (uintptr_t)ptr;
    assert((addr & SLAB_MASK) == 0);
    block_t *header = NULL;

    if(slab){
        header = (block_t *)ptr;
        assert(header->size == SLAB_PAGE); 
        assert(header->next == NULL);
        assert(header->magic == SLAB_MEM);
    }
    else{
        // big memory's header is a SLAB_PAGE before the ptr_addr
        header = (block_t*)((uintptr_t)ptr - SLAB_PAGE);
        assert((header->size & SLAB_MASK) == 0 && header->size >= SLAB_PAGE);

        if(header->magic != BIG_MEM){
            printf("ptr = %x size = %x magic = %x\n", ptr, header->size, header->magic);
        }
        assert(header->magic == BIG_MEM);
        assert(header->next == NULL);
        
        size_t align = align_up(header->size);
        assert(((uintptr_t)ptr & (align - 1)) == 0); 
        header->size += SLAB_PAGE;
    }

    header->magic = FREE_MEM;

    /*node_t** cursor = &central.head; // the point to node->next/central.head
    while(*cursor != NULL && (uintptr_t)(*cursor) < (uintptr_t)header){
        cursor = &(*cursor)->next;
    }
    header->next = *cursor; 
    *cursor = header;*/

    //insert and sorted by address
    block_t* cur = central.head;
    block_t* prev = NULL;
    while(cur != NULL && (uintptr_t)(cur) < (uintptr_t)header){
        prev = cur;
        cur = cur->next;
    }
    //printf("free: ptr = %x cur = %x prev = %x\n", (uintptr_t)ptr, (uintptr_t)cur, (uintptr_t)prev);

    if(prev == NULL){
        central.head = header;
    }
    else{
        prev->next = header;
        assert(prev->magic == FREE_MEM);
        assert((uintptr_t)prev < (uintptr_t)header && ((uintptr_t)prev + prev->size) <= (uintptr_t)header);
    }

    header->next = cur;

    if(header->next != NULL){
        if(header->next->magic != FREE_MEM){
            printf("%x size = %d\n",cur->next->magic, header->size);
        }
        assert(header->next->magic == FREE_MEM);
        assert((uintptr_t)header < (uintptr_t)cur && ((uintptr_t)header + header->size) <= (uintptr_t)(header->next));
    }
    

    // merge with the next block
    if(header->next != NULL && ((uintptr_t)header + header->size == (uintptr_t)header->next)){
        header->size += header->next->size;
        header->next = header->next->next;
    }

    // merge with the previous block
    if(prev != NULL && ((uintptr_t)prev + prev->size == (uintptr_t)header)){
        prev->size += header->size;
        prev->next = header->next;
    }

    spin_unlock(&central.lk);
}
