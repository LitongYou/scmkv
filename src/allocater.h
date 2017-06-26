#ifndef _ALLOCATER_H
#define _ALLOCATER_H

#include "config.h"
#include "page.h"
#include <cstring>
#include <sched.h>
#include "cpuinfo/cpu.h"

using namespace std;

//  1. cache on DRAM 
class Allocater {
    public:
        Allocater(int pageno_dzone = 0, int last_pageno = 0) {
            // creat new Allocater
            loger("Allocater: dzone start page #%d, end page #%d\n", pageno_dzone,
                    last_pageno);
            if (pageno_dzone) {
                memset(data_page, 0, sizeof(data_page));
                memset(bucket_page, 0, sizeof(bucket_page));
                _free_list_head = pageno_dzone;
                _free_list_tail = last_pageno;
                _last_pageno = last_pageno;
                _nr_pages =  last_pageno - pageno_dzone;
            }
        }

        void Redirect(PageTable *pt, Allocater *pm) { 
            _pt = pt;
            free_list_head = &pm->_free_list_head;
            free_list_tail = &pm->_free_list_tail;

            _pt->nth_to_page(*free_list_tail)->next = 0;
        }

        //malloc a space
        //return 0 if fail
        uint64_t malloc_generic(int slot, uint32_t size) 
        {
            slot = sched_getcpu();
            if (slot == -1)
                slot = 0;
            return malloc(&data_page[slot], size);
        }
        uint64_t malloc_bucket(int slot, uint32_t size) 
        {
            slot = sched_getcpu();
            if (slot == -1)
                slot = 0;
            return malloc(&bucket_page[slot], size);
        }

        //free a space
        void free(void *virt, uint32_t size);

    private:

        inline Page *page_on_dram(int *slot) {
            if (slot < bucket_page) 
                return &dpage_info[slot - data_page];
            return &bpage_info[slot - bucket_page];
        }

        void copy_page_from_pm(int *slot) {
            Page *pg = page_on_dram(slot);
            *pg = *(_pt->nth_to_page(*slot));
        }

        void copy_page_to_pm(int *slot) {
            Page *page_on_pm = _pt->nth_to_page(*slot);
            *page_on_pm = *page_on_dram(slot);
            clflush(page_on_pm, sizeof(Page));
        };

        uint64_t malloc(int *slot, uint32_t size);
        
        int alloc_page();
        void free_page(int pageno);

        PageTable *_pt;

        // a list of empty pages
        // atomic
        int *free_list_head;
        int *free_list_tail;
        // the following two fields
        int _free_list_head;
        int _free_list_tail;

        //int nr_concurrent;
        // following two fields are arrays, whose elements are pageno.
        // the number of elements in each array is nr_concurrent
        // atomic
        int data_page[kNrLogs];
        Page dpage_info[kNrLogs];
        int bucket_page[kNrLogs];
        Page bpage_info[kNrLogs];

        uint64_t _last_pageno;
        uint64_t _nr_pages;

};

#endif
