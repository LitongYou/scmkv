#include <iostream>
#include "atomic.h"


using namespace std;

void test(int64_t *d, int64_t *s, int64_t n)
{
    cout << "before test" << " " << *d << " " << *s << endl;
    bool b = atomic_compare_exchange(d, s, n);
    cout << "after test" << " " <<  *d << " " << *s << " " << b << endl;
}
int main()
{
    int64_t *dest = new int64_t(1);
    int64_t  src = 2;
    test(dest, &src, 3);

    src =  *dest;
    test(dest, &src, 3);
    return 0;
}


