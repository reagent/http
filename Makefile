CFLAGS=-Wall -g -I/usr/local/uriparser/include/uriparser
LDFLAGS=-L/usr/local/uriparser/lib -luriparser

all: http

http: buffer.o http.o

dev:CFLAGS=-DNDEBUG
dev: all

clean:
	rm -rf http *.o *.dSYM