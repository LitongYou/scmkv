#ifndef _ZONE_H
#define _ZONE_H

#include <cstdint>

struct Zone {
    uint64_t _virt;
    uint64_t _size;

    Zone(uint64_t virt, uint64_t size = 0) : _virt(virt), _size(size) {}

    inline uint64_t phy_to_virt(uint64_t phy) const;
    inline uint64_t virt_to_phy(uint64_t virt) const;

};

#endif
