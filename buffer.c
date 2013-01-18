#include "buffer.h"

Buffer *
buffer_alloc(int initial_size)
{
    Buffer *buf     = malloc(sizeof(Buffer));
    buf->contents   = calloc(1, initial_size * sizeof(char));
    buf->bytes_used = 0;
    buf->total_size = initial_size;

    return buf;
}

int
buffer_len(Buffer *buf)
{
    int len = buf->bytes_used;

    if (len > 0) { len++; } // Include space for NUL byte

    return len;
}

void
buffer_free(Buffer *buf)
{
    free(buf->contents);
    free(buf);
}

int
buffer_has_space(Buffer *buf, int desired_length)
{
    return desired_length <= (buf->total_size - buffer_len(buf));
}

int
buffer_grow(Buffer *buf, int minimum_size)
{
    int min_allocation = minimum_size + 1; // Need to hold NUL
    int new_size       = (buf->total_size * 2);

    if (new_size < min_allocation) {
        new_size = min_allocation;
    }

    char *tmp = realloc(buf->contents, new_size * sizeof(char));
    if (tmp == NULL) { return -1; }

    buf->contents   = tmp;
    buf->total_size = new_size;

    return 0;
}

void
buffer_cat(Buffer *buf, char *append, int length)
{
    int i               = 0;
    int bytes_copied    = 0;
    int buffer_position = 0;

    for (i = 0; i < length; i++) {
        if (append[i] == '\0') { break; }

        buffer_position = buf->bytes_used + i;
        *(buf->contents + buffer_position) = append[i];

        bytes_copied++;
    }

    buf->bytes_used += bytes_copied;
    *(buf->contents + buf->bytes_used) = '\0';
}

int
buffer_append(Buffer *buf, char *append, int length)
{
    int status = 0;

    if (!buffer_has_space(buf, length)) {
        status = buffer_grow(buf, length);
        if (status == -1) { return -1; }
    }

    buffer_cat(buf, append, length);

    return 0;
}
