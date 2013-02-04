#ifndef URL_H
#define URL_H

#include <Uri.h>
#include <buffer.h>

#include "dbg.h"

#define URL_PATH_BUFFER_SIZE 512
#define DEFAULT_PORT "80"

#define url_free_part(P) if(P) { free(P); }

typedef struct Url {
    char *scheme;
    char *hostname;
    char *port;
    char *path;
    char *query;
    char *fragment;
} Url;

Url *url_parse(char *url);
void url_free(Url *url);


#endif