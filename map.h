#ifndef MAP_H
#define MAP_H
#include <stdlib.h>
#define MAXLEN 100

typedef struct nlist{
     char *path;
     char *pagename;
     struct nlist * next;
}nlist; 

nlist **init(); // for initailizing the hashmap
unsigned hash(char*); // hash function
char *get_page_name(nlist ** , char *); // this returns a pagename with respect to specific path . If the path does not exist it returns NULL.
void route(nlist ** , char * , char *); // it maps section with the pagename
void destroylist(nlist**);  // for deallocating the nlist

#endif
