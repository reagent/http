CFLAGS=-static -Wall -g -I/usr/local/uriparser/include/uriparser -I./vendor/buffer/src
LDFLAGS=-L/usr/local/uriparser/lib -luriparser -L./vendor/buffer/build -lbuffer

SOURCES=$(wildcard src/**/*.c src/*.c)
OBJECTS=$(patsubst %.c,%.o,$(SOURCES))

TARGET=bin/http

all: $(TARGET)

$(TARGET): build $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $(OBJECTS)

build:
	@mkdir -p bin

dev:CFLAGS+=-DNDEBUG
dev: all

clean:
	rm -rf bin/http src/*.o *.dSYM