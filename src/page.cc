#include "page.h"
#include "memlayout.h"

Page *PageTable::virt_to_page(void *virt) 
{
    int no = ((uint64_t)virt - MemoryStorage::Mstore()->VirtBase()) / kPageSize;
    return _pages + no;
}

void *PageTable::nth_to_virt(int no) 
{
    return (void *)(MemoryStorage::Mstore()->VirtBase() + (uint64_t)no * kPageSize);
}

