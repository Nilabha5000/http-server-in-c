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


// this function sends a JavaScript file to the client    
void sendJS(int clientfd , char *filename){
      if(strstr(filename,".js") == NULL){
          perror("Provided file is not a javascript file");
          return;
      }
      FILE *jsFile = fopen(filename, "r");
    if (jsFile != NULL) {
        fseek(jsFile, 0, SEEK_END);
        long fileSize = ftell(jsFile);
        rewind(jsFile);

        char *buffer = malloc(fileSize + 1);
        fread(buffer, 1, fileSize, jsFile);
        buffer[fileSize] = '\0';
        fclose(jsFile);

        // Send response header
        send(clientfd, "HTTP/1.1 200 OK\r\n", strlen("HTTP/1.1 200 OK\r\n"), 0);
        send(clientfd, "Content-Type: application/javascript\r\n", strlen("Content-Type: application/javascript\r\n"), 0);
        send(clientfd, "Connection: close\r\n\r\n", strlen("Connection: close\r\n\r\n"), 0);

        // Send JS content
        int send_bytes  = send(clientfd, buffer, fileSize, 0);
        if (send_bytes < 0) {
            perror("Error sending JavaScript file");
        } else {
            printf("JavaScript file sent successfully.\n");
        }
        fflush(stdout);
        // Free allocated memory
        free(buffer);
    } else {
           /*    printf("No file exist \n ");
               char response[] = "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\nPage Not Found";
               send(clientfd, response, strlen(response), 0); */
    }

}
// this function sends a HTML file to the client
void sendHTML(int clientfd, char *filename){
      if(strstr(filename,".html") == NULL){
           perror("Provided file is not a html file");
          return;
      }
      
      FILE *html = fopen(filename, "r");
      if(html){
         char response_header[] = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
         send(clientfd, response_header, strlen(response_header), 0);

        char read_buffer[1024];
        while(fgets(read_buffer,1024,html)){
            send(clientfd , read_buffer, strlen(read_buffer), 0);
        }
         fclose(html);
      }
      else{
            printf("No file exist \n ");
            char response[] = "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\nPage Not Found";
            send(clientfd, response, strlen(response), 0);
      }
     
}
// this function sends a image file to the client
void sendImage(int clientfd , char *filename){
        if(strstr(filename,".jpg") == NULL){
            perror("Provided file is not a image file");
            return;
        }
      FILE *image = fopen(filename,"rb");
      char response_header[1024];
	  if(image){
           strcpy(response_header , "HTTP/1.1 200 OK\r\nContent-Type: image/jpeg\r\n\r\n");
            printf("%s\n", response_header);
            int send_bytes = send(clientfd, response_header, strlen(response_header), 0);
			if(send_bytes == 0){
                perror("Error to send headers to browser\n");
                return;
            }
			    char image_buffer[1024];
                size_t n;
			    while((n = fread(image_buffer,1,sizeof(image_buffer),image)) > 0){
                    send(clientfd, image_buffer, n , 0);
                    memset(image_buffer, 0, sizeof(image_buffer));
			    }
			       fclose(image);
            }
        else{
            printf("No file exist \n ");
            char response[] = "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\nPage Not Found";
            send(clientfd, response, strlen(response), 0);
        }
  
}
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
     //mapping path with html file , javascript file and image file
     route(mp, "/" , "index.html");
     route(mp,"/about", "about.html");
     route(mp,"/scene.jpg", "scene.jpg");
     route(mp,"/contact", "contact.html");
     route(mp,"/scene1","pexels-eberhardgross-1302242.jpg");
     route(mp,"/main.js", "main.js");
     printf(" / --> %s\n", get_page_name(mp ,"/"));
     printf(" /about --> %s\n", get_page_name(mp,"/about"));
     printf(" /contact --> %s\n", get_page_name(mp,"/contact"));
     printf(" /scene.jpg --> %s\n", get_page_name(mp,"/scene.jpg"));
     printf("/scene1 --> %s\n", get_page_name(mp,"/scene1"));
     printf(" /main.js --> %s\n", get_page_name(mp ,"/main.js"));
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
                // Check the file extension and call the appropriate function
                if(strstr(filename, ".js") != NULL){
                    sendJS(client_sock, filename);
                }
                else if(strstr(filename, ".html") != NULL){
                    sendHTML(client_sock, filename);
                }
                else if(strstr(filename, ".jpg") != NULL){
                    sendImage(client_sock, filename);
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
