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
    VirtDevice dev(1ul << 30);
    if (dev.Open() == -1)
        return -1;

    db_config cf = default_db_config;
    cf.force_create = true;

    DB db(dev, cf);
    string key, value;
    string ovalue;

    string ps1 = "admin@memkv / $ ";
    string cmd;
    cout << ps1;
    while (cin >> cmd) {
        if (cmd == "put") {
            cin >> key >> value;
            db.Put(key.c_str(), key.size(), value.c_str(), value.size());
        } else if (cmd == "get") {
            cin >> key;
            auto p = db.Get(key.c_str(), key.size());
            ovalue = "";
            if (p.first)
                ovalue = string(p.first, p.second);
            cout << ovalue << endl;
        } if (cmd == "del") {
            cin >> key;
            db.Delete(key.c_str(), key.size());
            //cout << endl;
        }
        cout << ps1;
    }
}


