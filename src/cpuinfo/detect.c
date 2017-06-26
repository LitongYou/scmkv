#include "cpu.h"

int main()
{
    int a[128];

    if (is_cpu_clwb_present())
        flush_clwb(a, sizeof(a));
    if (is_cpu_clflushopt_present())
        flush_clflushopt(a, sizeof(a));
    if (is_cpu_clflush_present())
        flush_clflush(a, sizeof(a));

    return 0;

}
