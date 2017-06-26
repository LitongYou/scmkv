#include "key.h"
#include "memlayout.h"

uint64_t Key::data_phy() const 
{ 
    return virt2phy(_data); 
}
