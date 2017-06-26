#include "zone.h"
#include "memlayout.h"


uint64_t Zone::phy_to_virt(uint64_t phy) const 
{ 
    return phy + MemoryStorage::Mstore()->VirtBase(); 
}
uint64_t Zone::virt_to_phy(uint64_t virt) const 
{
    return virt - MemoryStorage::Mstore()->VirtBase(); 
}
