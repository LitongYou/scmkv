#include <cstdio>
#include <cstdint>
#include "bitops.h"

int main()
{
    bool b;
    uint64_t n = 0xffffffffffffffff;
    printf(" %016lX\n %016lX\n %016lX\n\n", n, lowbits(48, n), highbits(16, n));

    n = 0xfff0ffffffffffff;
    printf(" %016lX\n %016lX\n %016lX\n\n", n, lowbits(48, n), highbits(16, n));
    
    n = 0xf0f77fffffffffff;
    printf(" %016lX\n %016lX\n %016lX\n\n", n, lowbits(48, n), highbits(16, n));

    n = 0xabf0ffffffffffff;
    printf(" %016lX\n %016lX\n %016lX\n\n", n, lowbits(56, n), highbits(8, n));

    n = 0x77f0ffffffffffff;
    printf(" %016lX\n %016lX\n %016lX\n\n", n, lowbits(56, n), highbits(8, n));

    n = 0xffffffffffffffff;
    printf(" %016lX\n", n);
    b = test_and_set_bit(63, &n);
    printf(" %016lX\n %d\n\n", n, b);

    n = 0x7fffffffffffffff;
    printf(" %016lX\n", n);
    b = test_and_set_bit(63, &n);
    printf(" %016lX\n %d\n\n", n, b);

    n = 0x11ffffffffffffff;
    printf(" %016lX\n", n);
    b = test_and_set_bit(56, &n);
    printf(" %016lX\n %d\n\n", n, b);

    n = 0x00ffffffffffffff;
    printf(" %016lX\n", n);
    b = test_and_set_bit(56, &n);
    printf(" %016lX\n %d\n\n", n, b);

    n = 0xffffffffffffffff;
    printf(" %016lX\n", n);
    b = test_bit(63, n);
    printf(" %016lX\n %d\n\n", n, b);

    n = 0x0fffffffffffffff;
    printf(" %016lX\n", n);
    b = test_bit(63, n);
    printf(" %016lX\n %d\n\n", n, b);

    n = 0xffffffffffffffff;
    printf(" %016lX\n", n);
    clear_bit(63, &n);
    printf(" %016lX\n %d\n\n", n, b);
    return 0;
}
