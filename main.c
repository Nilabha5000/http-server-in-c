#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include "map.h"
#include "httpserver.h"

http_server *server = NULL;
nlist **mp = NULL;
void handle_exit(int sigid) {
    if (server != NULL) {
        close(server->serverfd);
        free(server);
        destroylist(mp);
        printf("Exiting server successfully\n");
        server = NULL;
    }
    exit(0);
}


int main() {
    signal(SIGINT, handle_exit);
    signal(SIGTERM, handle_exit);
    //initializing the hashmap 
     mp = init();
     printf("\n-----------------------------------------------\n");
     //mapping path with html file , javascript file and image file
     route(mp, "/" , "test/index.html");
     route(mp,"/about", "test/about.html");
     route(mp,"/scene.jpg", "test/scene.jpg");
     route(mp,"/contact", "test/contact.html");
     route(mp,"/scene1","test/pexels-eberhardgross-1302242.jpg");
     route(mp,"/main.js", "test/main.js");
     route(mp,"/style1","test/style1.css");
     route(mp,"/photo.png", "test/photo.png");
     printf(" / --> %s\n", get_page_name(mp ,"/"));
     printf(" /about --> %s\n", get_page_name(mp,"/about"));
     printf(" /contact --> %s\n", get_page_name(mp,"/contact"));
     printf(" /scene.jpg --> %s\n", get_page_name(mp,"/scene.jpg"));
     printf("/scene1 --> %s\n", get_page_name(mp,"/scene1"));
     printf(" /main.js --> %s\n", get_page_name(mp ,"/main.js"));
     printf(" /photo.png --> %s\n", get_page_name(mp ,"/photo.png"));
     printf("---------------------------------------------------\n");
    server = init_server(8080);
    if (!server) {
        perror("Server init failed");
        return 1;
    }
    run(server->serverfd , mp);
    

    return 0;
}
