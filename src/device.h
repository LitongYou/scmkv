#ifndef _DEVICE_H
#define _DEVICE_H

#include <string>
#include <cstdlib>

using namespace std;

class Device {
    public:
        Device(string name, uint64_t offset, uint64_t length):
            _name(name), _offset(offset), _length(length){}
        ~Device() {};

        uint64_t BaseAddr() {return _addr; }
        uint64_t Size() { return _length; }

        virtual int Open();
        virtual int Close();

    protected:

        string _name;
        int _devno;
        uint64_t _offset, _length;
        uint64_t _addr;
};

class VirtDevice : public Device {
public:
	VirtDevice(uint64_t size): Device("", 0, size){}
	int Open() {
		_addr = (uint64_t) malloc(_length);
		return _addr;
	}
	int Close() {
		if (_addr != (uint64_t)-1)
			free((void *)_addr);
		return 0;
	}
};

#ifdef _HAVE_QUARTZ
#include "../src/lib/pmalloc.h"

class QuartzDevice : public Device {
public:
	QuartzDevice(uint64_t size): Device("", 0, size){}
	int Open() {
		_addr = (uint64_t) pmalloc(_length);
		return _addr;
	}
	int Close() {
		if (_addr != (uint64_t)-1)
			pfree((void *)_addr, _length);
		return 0;
	}
};
#endif


#endif

