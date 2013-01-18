#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>

#include "dbg.h"
#include "buffer.h"

#define RECV_SIZE 64
#define BUF_SIZE  RECV_SIZE + 1

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
build_request(char *hostname)
{
    char *request = malloc(128 * sizeof(char));
    memset(request, 0, 128);

    strncat(request, "GET / HTTP/1.0\r\nHost: ", 22);
    strncat(request, hostname, strlen(hostname) + 1);
    strncat(request, "\r\nConnection: close\r\n\r\n", 23);

    return request;
}

int
make_request(int sockfd, char *hostname)
{
    size_t bytes_sent = 0;
    char *request = build_request(hostname);

    // TODO: ensure we send entire request
    bytes_sent = send(sockfd, request, strlen(request), 0);

    free(request);

    return bytes_sent;
}

int
fetch_response(int sockfd, Buffer **response, int recv_size)
{
    size_t bytes_received;
    int status = 0;
    char data[recv_size];

    for (;;) {
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
main(int argc, char *argv[])
{

    Buffer *response = buffer_alloc(BUF_SIZE);

    int status = 0;
    int sockfd = 0;
    struct addrinfo *res = NULL;

    char hostname[64] = {'\0'};
    char port[6]      = "80";
    char ch;

    while ((ch = getopt(argc, argv, "h:p:")) != -1) {
        switch (ch) {
            case 'h':
                strncpy(hostname, optarg, 64);
                break;
            case 'p':
                strncpy(port, optarg, 6);
                break;
        }
    }

    error_unless(strlen(hostname) > 0, "Hostname not supplied");

    status = init_connection(hostname, port, &res);
    error_unless(status == 0, "Could not resolve host: %s\n", gai_strerror(status));

    sockfd = make_connection(res);
    error_unless(sockfd > 0, "Could not make connection to '%s' on port '%s'", hostname, port);

    status = make_request(sockfd, hostname);
    error_unless(status > 0, "Sending request failed");

    status = fetch_response(sockfd, &response, RECV_SIZE);
    error_unless(status >= 0, "Fetching response failed");

    printf("%s\n", response->contents);

    close(sockfd);

    freeaddrinfo(res);
    buffer_free(response);

    return 0;

error:
    if (sockfd > 0)  { close(sockfd); }
    if (res != NULL) { freeaddrinfo(res); }

    buffer_free(response);

    return 1;
}