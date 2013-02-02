#include <Uri.h>
#include <buffer.h>
#include "url.h"
#include "dbg.h"

int
length_of(UriTextRangeA *part)
{
    return part->afterLast - part->first;
}

char *
fetch_uri_part(UriTextRangeA *part)
{
    int length = length_of(part);

    if (length <= 0) { return NULL; }

    char *content = calloc(1, sizeof(char) * (length + 1));

    strncpy(content, part->first, length);

    return content;
}

char *
fetch_port(char *uri_port, char *default_port)
{
    return (uri_port == NULL || strlen(uri_port) == 0) ? default_port : uri_port;
}

UriUriA *
url_parse(char *url)
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

char *
url_path(UriUriA *uri)
{
    Buffer *path_buffer = buffer_alloc(URL_PATH_BUFFER_SIZE);
    UriPathSegmentA *loc = uri->pathHead;

    char *part = NULL,
         *path = NULL;

    while (loc != NULL) {
        part = fetch_uri_part(&loc->text);

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
