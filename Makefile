#CC=gcc
#CFLAGS=-Wall -o wrench -lpthread -std=c11
CC=clang++
CFLAGS=-Wall -o wrench -lpthread -std=c++11 -stdlib=libc++

all:
	$(CC) $(CFLAGS) main.cpp thread_serve.cpp

clean:
	rm *o wrench

run:
	./wrench -r test-repo
