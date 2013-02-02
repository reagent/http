#ifndef URL_H
#define URL_H

#define URL_PATH_BUFFER_SIZE 512

int length_of(UriTextRangeA *part);
char *fetch_uri_part(UriTextRangeA *part);
char *fetch_port(char *uri_port, char *default_port);
UriUriA *url_parse(char *url);
char *url_path(UriUriA *uri);

#endif