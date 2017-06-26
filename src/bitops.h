#ifndef _BITOPS_H
#define _BITOPS_H

#define BITS_PER_BYTE 8

template<typename T>
inline T aligned8(T num)
{
    return (((num + 7) >> 3) << 3);
}

template<typename T>
inline T lowbits(int order, T num)
{
    static_assert(sizeof(T) == 8,
            "low48bits only operator on 64-bit numbers");
    return num & ((1ul << order) - 1);
}

template<typename T>
inline T highbits(int order, T num)
{
    static_assert(sizeof(T) == 8,
            "highbits only operator on 64-bit numbers");
    return num >> ((sizeof(T) * BITS_PER_BYTE)  - order);
}

template<typename T>
inline bool test_bit(int order, T value) 
{
    return value & (1ul << order);
}

template<typename T>
inline void set_bit(int order, T *addr)
{
    *addr |= (1ul << order);
}

template<typename T, typename T2>
inline void set_high_bits(int nr_bits, T2 value, T *addr)
{
    static_assert(sizeof(T) == 8,
            "low48bits only operator on 64-bit numbers");
    int high_bit = sizeof(T) * 8 - nr_bits;
    *addr &= ((1ul << high_bit) - 1);
    *addr |= (value << high_bit);
}

template<typename T>
inline void clear_bit(int order, T *addr)
{
    *addr &= ~(1ul << order);
}

// set a bit and return its old value.
// This operation is atomic and implies a memory barrier.
template<typename T>
inline bool test_and_set_bit(uint64_t nr, T *addr)
{
    int oldbit;
    asm volatile("lock btsq %2, %1\n\t"
            "sbb %0, %0"
            :"=r"(oldbit),"+m"(*addr)
            :"Ir"(nr)
            :"memory");
    return oldbit;
}

#endif
