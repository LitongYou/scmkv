#include "allocater.h"
#include "atomic.h"
#include "config.h"
#include "cpuinfo/cpu.h"
#include <cassert>

int Allocater::alloc_page()
{
    int new_page = *free_list_head;
    
    if (_nr_pages < 5)
        return 0;

    int next = _pt->nth_to_page(new_page)->next;
    //printf("current %d next %d tail %d %lu\n",new_page,  next, free_list_tail, _last_pageno);

    while (!atomic_compare_exchange(free_list_head, &new_page, next)) {
        next = _pt->nth_to_page(new_page)->next;
        //printf("current %d next %d tail %d %lu\n",new_page,  next, free_list_tail, _last_pageno);
    }

    _pt->nth_to_page(new_page)->active = 1;
    _pt->nth_to_page(new_page)->used = 0;
    _pt->nth_to_page(new_page)->allocated = 0;

    //persist free_list_head on pm
    clflush(_pt->nth_to_page(new_page), sizeof(Page));

    __sync_fetch_and_sub(&_nr_pages, 1);

    return new_page;
}

void Allocater::free_page(int pageno)
{
    int last_page = *free_list_tail;
    _pt->nth_to_page(pageno)->next = 0;

    while (!atomic_compare_exchange(free_list_tail, &last_page, pageno))
        ;

    _pt->nth_to_page(last_page)->next = pageno;

    //persist free_list_tail on pm
    clflush(_pt->nth_to_page(last_page), sizeof(Page));

    __sync_fetch_and_add(&_nr_pages, 1);
}

uint64_t Allocater::malloc(int *slot, uint32_t size)
{
    assert(size % 8 == 0);

    if (*slot) {
        Page *pg = page_on_dram(slot);
        if (pg->allocated > kPageSize - size) {
            //persist page using clwb
            pg->active = 0;
            copy_page_to_pm(slot);
            *slot = 0;
        }
    }

    if (!*slot) {
        *slot = alloc_page();
        copy_page_from_pm(slot);
    }

    if (*slot) {
        Page *pg = page_on_dram(slot);
        char *p = (char *)(_pt->nth_to_virt(*slot)) + pg->allocated;
        pg->allocated += size;
        pg->used += size;
        //*p = 1;
        //printf("allocate %p end %p pg %d total %lu\n", p, p + size,  *slot, _last_pageno);
        return (uint64_t)p; 
    }
    printf("allocate error\n");
    return 0;
}

void Allocater::free(void *virt, uint32_t size)
{
    Page *pg = _pt->virt_to_page(virt);
    
    __sync_fetch_and_sub(&(pg->used), size);
    clflush(pg, sizeof(*pg));
    
    if (!(pg->active) && !(pg->used)) {
        free_page(_pt->page_to_nth(pg));
    }
    return ;
}
