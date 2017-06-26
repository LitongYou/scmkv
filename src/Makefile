CC = g++ -std=c++14 -g -Wall -mavx -maes -DNDEBUG -DCAP_CLNONE -march=native
CC = clang++ -std=c++14 -g  -Wall -mavx -maes -fPIC
CC = g++ -std=c++14 -g  -Wall -mavx -maes -fPIC
CC = g++ -std=c++14 -O2 -Wall -mavx -maes -DNDEBUG -DCAP_CLNONE -march=native

LIBS = -lcrypto -lpthread

LIBRARY = gen.o key.o memlayout.o db.o zone.o device.o page.o allocater.o page.o htable.o dzone.o memlayout.o hash/farmhash.o key.o cpuinfo/cpu.o

lib: libscmkv.a

db: manual.o libscmkv.a
	$(CC) -o $@ $^ $(LIBS)

libscmkv.a: $(LIBRARY)
	ar rcs libscmkv.a $^

.cc.o:
	$(CC) -c -o $@  $<

.PHONY: clean

clean:
	$(RM) *.o *.a a.out db
