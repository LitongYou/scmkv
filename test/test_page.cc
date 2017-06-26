#include <cstdio>
#include <cstdint>
#include <atomic>

using namespace std;
struct Page {
    uint32_t free:1; // is this page free
    uint32_t aed:24; // is this page free
    union {
        uint32_t used:24; // the size of data in a page must be less than 16M
        uint32_t next:24; // the number of pages in this system must be less than 16M
    };
};

int main()
{
    atomic<Page> page[16];
    atomic<Page> *p;
    p = page + 8;
    if (page[0].is_lock_free())
        printf("lock free\n");
    printf("%d, %d, %d\n", sizeof(page[0]), sizeof(p[0]), p - page);
    return 0;
}
