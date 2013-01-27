#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>

#include <Uri.h>

#include "dbg.h"
#include "buffer.h"

#define RECV_SIZE 1024
#define BUF_SIZE  RECV_SIZE + 1
#define DEFAULT_PORT "80"

int
init_connection(char *hostname, char *port, struct addrinfo **res)
{
    struct addrinfo hints;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    return getaddrinfo(hostname, port, &hints, res);
}

int
make_connection(struct addrinfo *res)
{
    int sockfd, status;

    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    jump_unless(sockfd > 0);

    status = connect(sockfd, res->ai_addr, res->ai_addrlen);
    jump_unless(status == 0);

    return sockfd;

error:
    return -1;
}

char *
build_request(char *hostname, char *request_path)
{
    char *request = calloc(1, 256 * sizeof(char)); // TODO: Use buffer here

    strncat(request, "GET ", 4);
    strncat(request, request_path, strlen(request_path));
    strncat(request, " HTTP/1.0\r\nHost: ", 19);
    strncat(request, hostname, strlen(hostname));
    strncat(request, "\r\nConnection: close\r\n\r\n", 23);

    return request;
}

int
make_request(int sockfd, char *hostname, char *request_path)
{
    char *request           = build_request(hostname, request_path);
    size_t bytes_sent       = 0;
    size_t total_bytes_sent = 0;
    size_t bytes_to_send    = strlen(request);


    debug("Bytes to send: %ld", bytes_to_send);

    while (1) {
        bytes_sent = send(sockfd, request, strlen(request), 0);
        total_bytes_sent += bytes_sent;

        debug("Bytes sent: %ld", bytes_sent);

        if (total_bytes_sent >= bytes_to_send) {
            break;
        }
    }

    free(request);

    return total_bytes_sent;
}

int
fetch_response(int sockfd, Buffer **response, int recv_size)
{
    size_t bytes_received;
    int status = 0;
    char data[recv_size];

    while (1) {
        bytes_received = recv(sockfd, data, RECV_SIZE, 0);

        if (bytes_received == -1) {
            return -1;
        } else if (bytes_received == 0) {
            return 0;
        }

        if (bytes_received > 0) {
            buffer_append(*response, data, bytes_received);
        }
    }

    return status;
}

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
fetch_port(char *uri_port, char *default_port) {
    return (uri_port == NULL || strlen(uri_port) == 0) ? default_port : uri_port;
}

UriUriA *
parse_url(char *url)
{
    UriParserStateA state;
    UriUriA *uri = malloc(sizeof(UriUriA));

    state.uri = uri;
    if (uriParseUriA(&state, url) != URI_SUCCESS) {
        uriFreeUriMembersA(uri);
        free(uri);

        return NULL;
    }

    return uri;
}

void
fetch_path(Buffer **path, UriUriA *uri)
{
    UriPathSegmentA *loc = uri->pathHead;

    char *part = NULL;

    while (loc != NULL) {
        part = fetch_uri_part(&loc->text);

        buffer_append(*path, "/", 1);
        buffer_append(*path, part, length_of(&loc->text));

        free(part);

        loc = loc->next;
    }

    if (buffer_len(*path) == 0) {
        buffer_append(*path, "/", 1);
    }
}

int
main(int argc, char *argv[])
{
    Buffer *response = buffer_alloc(BUF_SIZE);
    Buffer *path     = buffer_alloc(BUF_SIZE);

    int status = 0;
    int sockfd = 0;
    struct addrinfo *res = NULL;

    char *hostname, *port_from_uri, *port;

    if (argc != 2) {
        fprintf(stderr, "Usage: http <url>\n");
        buffer_free(response);
        return 1;
    }

    UriUriA *uri = parse_url(argv[1]);

    hostname      = fetch_uri_part(&uri->hostText);
    port_from_uri = fetch_uri_part(&uri->portText);

    fetch_path(&path, uri);

    port = fetch_port(port_from_uri, DEFAULT_PORT)

    debug("Using '%s' as hostname, '%s' for port", hostname, port);

    status = init_connection(hostname, port, &res);
    error_unless(status == 0, "Could not resolve host: %s\n", gai_strerror(status));

    sockfd = make_connection(res);
    error_unless(sockfd > 0, "Could not make connection to '%s' on port '%s'", hostname, port);

    status = make_request(sockfd, hostname, path->contents);
    error_unless(status > 0, "Sending request failed");

    status = fetch_response(sockfd, &response, RECV_SIZE);
    error_unless(status >= 0, "Fetching response failed");

    printf("%s\n", response->contents);

    close(sockfd);

    freeaddrinfo(res);
    uriFreeUriMembersA(uri);
    buffer_free(response);
    buffer_free(path);
    free(hostname);
    free(port_from_uri);
    free(uri);
    return 0;

error:
    if (sockfd > 0)  { close(sockfd); }
    if (res != NULL) { freeaddrinfo(res); }

    uriFreeUriMembersA(uri);
    buffer_free(response);
    buffer_free(path);
    free(hostname);
    free(port_from_uri);
    free(uri);

    return 1;
}