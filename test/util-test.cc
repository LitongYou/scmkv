#include <iostream>
#include <cstdio>
#include "utils.h"

using namespace std;

int main ()
{
    using list_t = buffered_list<int>;
    using node_t = list_t::node_type;
    using mem_t = buffer<node_t>;

    mem_t pool(500);
    list_t ls;
    ls.set_buffer(&pool);

    ls.lock();
    printf("%p %p\n", ls.begin(), ls.end());
    ls.lock();
    cout << "try lock twice" << endl;
    return 0;

    for (int i = 0; i < 500; i ++ ) {
        ls.push_front(500 - i);
    }
    cout << "after insert" << endl;
    printf("%p %p\n", ls.begin(), ls.end());
    return 0;
    printf("%p %p\n", ls.begin(), ls.end());
    for (int *p = ls.begin(); p != ls.end(); p = ls.next(p)) {
        cout << *p << " ";
    }
    cout << endl;
    for (int i = 0; i < 500; i ++ ) {
        ls.remove(i);
    }
    cout << "after remove" << endl;
    for (int *p = ls.begin(); p != ls.end(); p = ls.next(p)) {
        cout << *p << " ";
    }
    cout << endl;

    for (int i = 0; i < 500; i ++ ) {
        ls.push_front(i);
    }
    cout << "after insert" << endl;
    for (int *p = ls.begin(); p != ls.end(); p = ls.next(p)) {
        cout << *p << " ";
    }
    cout << endl;
    for (int i = 0; i < 500; i ++ ) {
        ls.remove(i);
    }
    cout << "after remove" << endl;
    for (int *p = ls.begin(); p != ls.end(); p = ls.next(p)) {
        cout << *p << " ";
    }
    cout << endl;

    for (int i = 0; i < 500; i ++ ) {
        ls.push_front(500 - i);
    }
    cout << "after insert" << endl;
    for (int *p = ls.begin(); p != ls.end(); p = ls.next(p)) {
        cout << *p << " ";
    }
    cout << endl;
    for (int i = 0; i < 500; i ++ ) {
        ls.remove(500 - i);
    }
    cout << "after remove" << endl;
    for (int *p = ls.begin(); p != ls.end(); p = ls.next(p)) {
        cout << *p << " ";
    }
    cout << endl;

    return 0;

}
