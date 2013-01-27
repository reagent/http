#ifndef BUFFER_H
#define BUFFER_H

#include <stdlib.h>
#include <strings.h>

struct Buffer {
    char *contents;
    int bytes_used;
    int total_size;
};

typedef struct Buffer Buffer;

Buffer * buffer_alloc(int initial_size);
int buffer_len(Buffer *buf);
void buffer_free(Buffer *str);
int buffer_append(Buffer *str, char *append, int length);

#endif