#include <cstdio>
#include <cstring>
#include <cstdint>
#include <string>
#include <thread>
#include <queue>
#include <fcntl.h>
#include <unistd.h>

#include "config.h"
#include "device.h"
#include "db.h"
#include "key.h"

#include <iostream>
using namespace std;

int main(int argc, char *argv[])
{
    Device dev("/dev/mem", 2ul << 30, 1ul << 30);
    if (dev.Open() == -1)
        return -1;

    db_config cf = default_db_config;
    if (argc > 1 && !memcmp(argv[1], "-n", 2)) {
        cf.force_create = true;
    }

    DB db(dev, cf);
    string key, value;
    string ovalue;

    cout << "new key-vaule:";
    while (cin >> key >> value) {

      // db.Put(key, value);
       sleep(1);
       //db.Get(key, &ovalue);

       cout << key <<  ":" << ovalue << endl;
       cout << " " <<  endl;;
       cout << "new key-vaule:";

    }
}


