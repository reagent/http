#include <stdlib.h>
#include <stdio.h>

#include "url.h"
#include "connect.h"

#include "dbg.h"
#include <buffer.h>

int
main(int argc, char *argv[])
{
    Buffer *response = buffer_alloc(BUF_SIZE);
    Url *url;

    int status = 0;
    int sockfd = 0;
    struct addrinfo *res = NULL;

    if (argc != 2) {
        fprintf(stderr, "Usage: http <url>\n");
        buffer_free(response);
        return 1;
    }

    url = url_parse(argv[1]);
    error_unless(url, "Invalid URL supplied: '%s'", argv[1]);

    status = init_connection(url->hostname, url->port, &res);
    error_unless(status == 0, "Could not resolve host: %s\n", gai_strerror(status));

    sockfd = make_connection(res);
    error_unless(sockfd > 0, "Could not make connection to '%s' on port '%s'", url->hostname, url->port);

    status = make_request(sockfd, url->hostname, url->path);
    error_unless(status > 0, "Sending request failed");

    status = fetch_response(sockfd, &response, RECV_SIZE);
    error_unless(status >= 0, "Fetching response failed");

    printf("%s\n", response->contents);

    close(sockfd);

    freeaddrinfo(res);
    buffer_free(response);
    url_free(url);

    return 0;

error:
    if (sockfd > 0)  { close(sockfd); }
    if (res != NULL) { freeaddrinfo(res); }

    if (url) { url_free(url); }

    buffer_free(response);

    return 1;
}