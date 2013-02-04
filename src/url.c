#include "url.h"

char *uri_fetch_path(UriUriA *uri);
int length_of(UriTextRangeA *part);
char *uri_fetch_part(UriTextRangeA *part);
char *uri_fetch_port(UriTextRangeA *part);
UriUriA *uri_parse(char *url);
int url_check(Url *url);

Url *
url_parse(char *url_to_parse)
{
    Url *url = malloc(sizeof(Url));
    memset(url, 0, sizeof(Url));

    UriUriA *uri = uri_parse(url_to_parse);

    url->scheme   = uri_fetch_part(&uri->scheme);
    url->hostname = uri_fetch_part(&uri->hostText);
    url->port     = uri_fetch_port(&uri->portText);
    url->path     = uri_fetch_path(uri);

    // TODO: handle query & fragment

    debug("Scheme: '%s', Hostname: '%s', Port: '%s', Path: '%s'", url->scheme, url->hostname, url->port, url->path);

    jump_unless(url_check(url) == 0);

    uriFreeUriMembersA(uri);
    free(uri);

    return url;
error:
    uriFreeUriMembersA(uri);
    free(uri);
    url_free(url);

    return NULL;
}

int
url_check(Url *url)
{
    int invalid = 0;

    invalid = invalid || url->scheme == NULL || strcmp(url->scheme, "http");
    invalid = invalid || url->hostname == NULL;

    return invalid;
}

void
url_free(Url *url)
{
    url_free_part(url->scheme);
    url_free_part(url->hostname);
    url_free_part(url->port);
    url_free_part(url->path);
    url_free_part(url->query);
    url_free_part(url->fragment);

    free(url);
}

char *
uri_fetch_path(UriUriA *uri)
{
    Buffer *path_buffer = buffer_alloc(URL_PATH_BUFFER_SIZE);
    UriPathSegmentA *loc = uri->pathHead;

    char *part = NULL,
         *path = NULL;

    while (loc != NULL) {
        part = uri_fetch_part(&loc->text);

        buffer_nappendf(path_buffer, length_of(&loc->text) + 1, "/%s", part);

        free(part);

        loc = loc->next;
    }

    if (buffer_strlen(path_buffer) == 0) {
        buffer_append(path_buffer, "/", 1);
    }

    path = buffer_to_s(path_buffer);

    buffer_free(path_buffer);

    return path;
}
int
length_of(UriTextRangeA *part)
{
    return part->afterLast - part->first;
}

char *
uri_fetch_part(UriTextRangeA *part)
{
    char *contents = NULL;
    int length     = length_of(part);

    if (length > 0) {
        contents = calloc(1, sizeof(char) * (length + 1));
        strncpy(contents, part->first, length);
    }

    return contents;
}

char *
uri_fetch_port(UriTextRangeA *part)
{
    char *port = uri_fetch_part(part);

    if (port == NULL) {
        port = calloc(1, (strlen(DEFAULT_PORT) + 1) * sizeof(char));
        strncpy(port, DEFAULT_PORT, strlen(DEFAULT_PORT));
    }

    return port;
}

UriUriA *
uri_parse(char *url)
{
    int status;
    UriParserStateA state;
    UriUriA *uri = malloc(sizeof(UriUriA));

    state.uri = uri;

    status = uriParseUriA(&state, url);
    jump_unless(status == URI_SUCCESS);

    status = uriNormalizeSyntaxExA(uri, URI_NORMALIZE_PATH);
    jump_unless(status == URI_SUCCESS);

    return uri;
error:
    uriFreeUriMembersA(uri);
    free(uri);

    return NULL;
}