CC = g++ -std=c++14 -g -Wall -mavx -maes -DNDEBUG -DCAP_CLNONE -march=native
CC = g++ -std=c++14 -O2 -Wall -mavx -maes -DNDEBUG -DCAP_CLNONE -march=native

LIBS = -lcrypto -lpthread -L./../src/ -lscmkv 
INCLUDES = -I../src/

bin =  db-test

all: $(bin)

db-test: db-test.o $(LIBRARY) 
	$(CC)  -o $@ $^  $(LIBS) 

.cc.o:
	$(CC) -c -o $@ $< $(INCLUDES)

.PHONY: clean

clean:
	$(RM) *.o $(LIBRARY) $(bin) a.out
