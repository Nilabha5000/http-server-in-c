#include "map.h"
#include <string.h>

nlist **init(){
    nlist **res = (nlist**)malloc(sizeof(nlist*)*MAXLEN);
    
    for(int i = 0; i < MAXLEN; ++i)
       res[i] = NULL;
    return res;
}

unsigned hash(char *s){
       unsigned hashval;
       for(hashval = 0 ; *s != '\0'; s++)
        {
             hashval = *s + 31 *hashval;
        }
     return hashval % MAXLEN;
}

void route(nlist **list , char *path , char *pagename){
      unsigned index = hash(path);
      //allocating a new node 
      nlist *np = NULL;
      np = (nlist*)malloc(sizeof(nlist));
      np->next = NULL;
      np->path = (char*)malloc(sizeof(char)*strlen(path));
      np->pagename = (char*)malloc(sizeof(char)*strlen(pagename));
      strcpy(np->path, path);
      strcpy(np->pagename , pagename);
      
     // if the list index is empty then we assign it with np
      if(list[index] == NULL){
           list[index] = np;
           return;
      }

      //adding the node at last of list index
      nlist *i = NULL;
      for(i = list[index]; i->next != NULL ; i = i->next);
      i->next = np;
      
}

char *get_page_name(nlist**list , char *path){
     unsigned index = hash(path);
         for(nlist *i = list[index]; i != NULL; i = i->next){
              if(strcmp(i->path , path) == 0){
                     return i->pagename;
               }
         }
      return NULL;
}


void destroylist(nlist ** list){
     //code for deallocating list
     for(int i = 0; i < MAXLEN; ++i){
           //if list[i] is not NULL then deallocate all the nodes of list[i]
           if(list[i] != NULL){
                 nlist *curr = list[i];
                 nlist *prev = NULL;
                 while(curr != NULL){
                    prev = curr;
                    free(curr->path);
                    free(curr->pagename);
                    curr = curr->next;
                    free(prev);
                 }
           }
      }
     free(list);

}
