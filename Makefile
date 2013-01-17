CFLAGS=-Wall -g

all: http

http: buffer.o http.o

clean:
	rm -rf http *.o *.dSYM