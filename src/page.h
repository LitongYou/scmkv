#ifndef _PAGE_H
#define _PAGE_H

#include <cassert>

#include "zone.h"
#include "config.h"
#include "log.h"
#include <atomic>

using namespace std;

extern const int kPageSize;

// sizeof(Page) is 8
// TODO:
// add @nr_items to indicate the number of valid items
struct Page {
    uint32_t active:1; // is this page active
    uint32_t lock:1;
    uint32_t allocated:24; //don't decrease this.

    union {
        uint32_t used; // the size of data in a page must be less than 16M
        uint32_t next; // the number of pages in this system must be less than 2^32
    };

    operator uint64_t() {
        return *reinterpret_cast<uint64_t *>(this);
    }
};

// all pages in the system
class PageTable : private Zone {
    public:
        PageTable(Zone z):
            Zone(z), 
            _pages((Page *)_virt),
            _nr_pages(_size / sizeof(Page)){
                loger("PageTable(): start %lX, end %lX, _nr_pages %d\n",
                        _virt, _virt + _size, _nr_pages);
            }

        Page *nth_to_page(int no) { 
            assert(no < _nr_pages);
            return _pages + no;
        } 

        int page_to_nth(Page *page) {
            assert(page < _pages + _nr_pages);
            return page - _pages;
        }

        Page* virt_to_page(void *virt);

        void* nth_to_virt(int no);

    private:
        Page *_pages;
        int _nr_pages;
};

#endif

