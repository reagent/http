CFLAGS=-Wall -g

all: http

http: buffer.o http.o

dev:CFLAGS=-DNDEBUG
dev: all

clean:
	rm -rf http *.o *.dSYM