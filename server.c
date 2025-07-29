#include <stdio.h>
#include "map.h"
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>


int sockfd = -1; // Socket file descriptor
nlist **mp = NULL; // Hashmap pointer

void handle_exit(int sig_id){
      printf("\nExiting server ...\n");
      close(sockfd);
      destroylist(mp);
      printf("Server closed successfully.\n");
      fflush(stdout);
      exit(0);
}
int main(){

    signal(SIGINT,handle_exit);
    signal(SIGTERM, handle_exit);
    struct sockaddr_in server_addr;
     int port = 8080;
    // Create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
     int opt = 2;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
     //initializing the hashmap 
     mp = init();
     printf("\n-----------------------------------------------\n");
     //mapping path with html file
     route(mp, "/" , "index.html");
     route(mp,"/about", "about.html");
     route(mp,"/scene.jpg", "scene.jpg");
     route(mp,"/contact", "contact.html");
     printf(" / --> %s\n", get_page_name(mp ,"/"));
     printf(" /about --> %s\n", get_page_name(mp,"/about"));
     printf(" /contact --> %s\n", get_page_name(mp,"/contact"));
     printf(" /scene.jpg --> %s\n", get_page_name(mp,"/scene.jpg"));
     printf("---------------------------------------------------\n");
    // Set up the server address structure
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port); // Port number
    server_addr.sin_addr.s_addr = INADDR_ANY;// Localhost

    bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    listen(sockfd,5);
    printf("server is listeing on port %d\n", port);
    socklen_t addr_len = sizeof(server_addr);
    int client_sock;
    char buffer[4096] = {0};
    char method[8] = {0} , path[1024] = {0};
    char *filename = NULL;
    while(1){
        client_sock = accept(sockfd, (struct sockaddr *)&server_addr, &addr_len);
    if(client_sock < 0){
          perror("error \n");
          return 1;
    }
    printf("client connected \n");
    
        int recv_bytes = recv(client_sock,buffer,4096,0);
        printf("%s\n",buffer);
        if(recv_bytes == 0){
             printf("no bytes are received\n");
        }
        sscanf(buffer,"%s %s",method,path);
  
           if((filename = get_page_name(mp,path)) != NULL){
                 printf("rendered files is %s\n",filename);
                 char *ext = strstr(filename,".jpg");
                 int send_bytes = 0;
                 char response_header[1024];
                 if(ext != NULL){
			   FILE *image = fopen(filename,"rb");
			   if(image){
                    strcpy(response_header , "HTTP/1.1 200 OK\r\nContent-Type: image/jpeg\r\n\r\n");
                    printf("%s\n", response_header);
                    send_bytes = send(client_sock, response_header, strlen(response_header), 0);
			       if(send_bytes == 0){
                        perror("Error to send headers to browser\n");
                        return EXIT_FAILURE;
                    }
			       char image_buffer[1024];
                   size_t n;
			       while((n = fread(image_buffer,1,sizeof(image_buffer),image)) > 0){
                        send(client_sock, image_buffer, n , 0);

                        memset(image_buffer, 0, sizeof(image_buffer));
			       }
			       fclose(image);
			    }
                    }
                    else{
			          FILE *html = fopen(filename,"r");
			          if(html){

                           strcpy(response_header, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n");
                            printf("%s\n", response_header);
                           send_bytes = send(client_sock, response_header, strlen(response_header), 0);
                           if(send_bytes == 0){
                              perror("Error to send headers to browser\n");
                              return EXIT_FAILURE;
                           }
                           char read_buffer[1024];
                          while(fgets(read_buffer, 1024, html)){
                           //printf("%s\n", read_buffer);
                           send(client_sock, read_buffer,strlen(read_buffer) , 0);
                           memset(read_buffer,0,1024);
                       }
                        fclose(html);
                      }
		   }
           }
          else{
               printf("No file exist \n ");
               char response[] = "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\nPage Not Found";
               send(client_sock, response, strlen(response), 0);
          }
       memset(buffer,0,4096);
       memset(method,0,8);
       memset(path,0,1024);
       close(client_sock);
    }
}
