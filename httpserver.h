#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include "map.h"
#include <stdio.h>
#include <stdlib.h>
typedef struct http_server{
     int serverfd;
     int port;
}http_server;

typedef struct MIME{
    char *extention;
    char *content_type;
}MIME;

typedef struct pair{
    int *fd;
    nlist **mp;
}pair;


http_server *init_server(int);
void sendfile(char * , int);
void handle_client(void *);
void run(int , nlist **);
#endif
