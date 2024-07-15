CFLAGS=-static -Wall -g -I/usr/local/include/uriparser/ -I./vendor/buffer/src
LDFLAGS=-L/usr/local/lib/ -L./vendor/buffer/build -lbuffer -luriparser 

SOURCES=$(wildcard src/**/*.c src/*.c)
OBJECTS=$(patsubst %.c,%.o,$(SOURCES))

TARGET=bin/http

all: $(TARGET)

$(TARGET): build $(OBJECTS)
	$(CC) $(OBJECTS) $(LDFLAGS)  -o $@ 

build:
	@mkdir -p bin

dev:CFLAGS+=-DNDEBUG
dev: all

clean:
	rm -rf bin/http src/*.o *.dSYM
