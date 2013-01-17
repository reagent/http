#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>

#include "buffer.h"

#define RECV_SIZE 64
#define BUF_SIZE  RECV_SIZE + 1

int
init_connection(char *hostname, char *port, struct addrinfo **res)
{
    int status = 0;
    struct addrinfo hints;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    status = getaddrinfo(hostname, port, &hints, res);
    if (status != 0) {
        printf("Could not resolve host: %s\n", gai_strerror(status));
        return -1;
    }

    return status;
}

int
make_connection(struct addrinfo *res)
{
    int sockfd, status;

    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sockfd == -1) {
        printf("Could not initialize socket\n");
        return -1;
    }

    status = connect(sockfd, res->ai_addr, res->ai_addrlen);
    if (status == -1) {
        printf("Could not connect to host\n");
        return status;
    }

    return sockfd;
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

    if (strlen(hostname) == 0) {
        printf("Hostname not supplied\n");
        goto error;
    }

    status = init_connection(hostname, port, &res);
    if (status < 0 ) { goto error; }

    sockfd = make_connection(res);
    if (sockfd < 0) { goto error; }

    status = make_request(sockfd, hostname);
    if (status < 0) {
        printf("Request failed.\n");
        goto error;
    }

    status = fetch_response(sockfd, &response, RECV_SIZE);
    if (status < 0) {
        printf("Fetching response failed.\n");
        goto error;
    }

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