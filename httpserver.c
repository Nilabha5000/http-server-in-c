#include "httpserver.h"
#include "map.h"
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
//for MIME types 
const MIME mime_types[]={
        {".js" , "application/javascript"},
        {".html" , "text/html"},
        {".css" , "text/css"},
        {".txt", "text/plain"},
        {".png" , "image/png"},
        {".jpeg" , "image/jpeg"},
        {".jpg" , "image/jpeg"}
};
// Function to initialize the HTTP server
http_server *init_server(int port){
   http_server *hts = (struct http_server*)malloc(sizeof(http_server));
   hts->serverfd = socket(AF_INET , SOCK_STREAM , 0);
   if(hts->serverfd < 0){
        perror("Failed to initialize socket");
        free(hts);
        return NULL;
   }
   hts->port = port;
   int opt = 2;
    setsockopt(hts->serverfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
   struct sockaddr_in server_addr;
   server_addr.sin_family = AF_INET;
   server_addr.sin_port = htons(hts->port);
   server_addr.sin_addr.s_addr = INADDR_ANY;

   if(bind(hts->serverfd , (struct sockaddr *)&server_addr , sizeof (server_addr)) < 0){
        perror("failed to bind ");
        free(hts);
        return NULL;
   }
   if(listen(hts->serverfd , 20) < 0){
      perror("failed to listen");
      free(hts);
      return NULL;
   }
   
   printf("server is listeing on port %d\n", port);
   return hts;
}
// Function to get the file extension from a filename
char *ends_with(char *filename){
    if(filename == NULL) return NULL;
    int n = strlen(filename)-1;
    
    while(n > 0){
            if(filename[n] == '.')
		 break;
	    n--;
   }
   if(n == 0) return NULL;
   return filename + n;
}
// Function to get the content type based on the file extension
char *get_content_type(char *filename){
      

    char *extention = ends_with(filename);
    printf("%s\n",extention);
    if(extention == NULL) return NULL;
    
    for(int i = 0; i < 7; ++i){
         if(strcmp(extention , mime_types[i].extention) == 0){
              printf("\n%s\n",mime_types[i].content_type);
              return mime_types[i].content_type;
         }
    }
   return NULL;
}
// Function is suitable for sending text-based files
// such as HTML, CSS, JS, and TXT files.
// It reads the file, sends its content to the client, and handles errors.
// It also ensures that the file is properly closed after sending.
// The function allocates memory for the file content and frees it after sending.
void send_text_based_files(char *filename , int clientfd){
     FILE *File = fopen(filename, "r");
      if (File != NULL) {
        fseek(File, 0, SEEK_END);
        long fileSize = ftell(File);
        rewind(File);

        char *buffer = (char*)malloc(fileSize + 1);
        fread(buffer, 1, fileSize, File);
        buffer[fileSize] = '\0';
        fclose(File);
     int send_bytes  = send(clientfd, buffer, fileSize, 0);
        if (send_bytes < 0) {
            perror("Error sending file");
        } else {
            printf("file sent successfully.\n");
        }
        fflush(stdout);
        // Free allocated memory
        free(buffer);
    }
}
// Function to send image-based files
// such as JPEG and PNG files.
// It reads the file in binary mode, sends its content to the client, and handles errors.
// The function does not allocate memory for the file content, it reads directly into a buffer.
// It ensures that the file is properly closed after sending.    
void send_image_based_files(char *filename , int clientfd){
        FILE *image = fopen(filename,"rb");
        if(!image){
             perror("failed to open this image file");
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
// Function to send the file to the client
// It checks if the file exists, retrieves its content type, and sends the appropriate HTTP response.
// If the file does not exist, it sends an error message.
// It handles different file types (text-based and image-based) accordingly.
void sendfile(char* filename , int clientfd){
       if(filename == NULL){
            perror("requested file does not exist ");
            return;
       }
       char httpHeader[413] = "HTTP/1.1 200 OK\r\nContent-type: ";
       char *content_type = get_content_type(filename);
       if(content_type == NULL){
           perror("no content exist!");
           return;
       }
       char *ext = ends_with(filename);
       strcat(httpHeader , content_type);
       strcat(httpHeader, "\r\n\r\n");
       printf("\n%s\n",httpHeader);
       send(clientfd , httpHeader , strlen(httpHeader), 0);
       if(strcmp(ext,".js") == 0 || strcmp(ext,".css") == 0 || strcmp(ext,".html") == 0 || strcmp(ext,".txt") == 0)
         send_text_based_files(filename , clientfd);
       else if(strcmp(ext , ".jpeg") == 0 || strcmp(ext , ".jpg") == 0 || strcmp(ext, ".png") == 0)
         send_image_based_files(filename , clientfd);
       
} 
// Function to handle the client request
// It receives the request, extracts the method and path, and sends the requested file to the client.
// It uses the get_page_name function to retrieve the page name based on the path.
void handle_client(void *arg){
     pair *p = (pair*)arg;
     printf("%s\n",get_page_name(p->mp, "/"));
     char buffer[4096] = {0};
     char method[8] = {0} , path[20] = {0};
     recv(*p->fd, buffer , 4096 , 0);
     printf("%s\n",buffer);
     sscanf(buffer ,"%s %s",method, path);
     
    printf("Method : %s\n", method);
    printf("path : %s\n",path);
    sendfile(get_page_name(p->mp, path) , *p->fd);
}
void *threaded_client_handler(void *arg) {
    pair *p = (pair *)arg;
    int clientfd = *p->fd;
    
   
    handle_client(p);
    close(clientfd); // Don't forget to close the client socket
    free(p->fd); // Free the dynamically allocated socket descriptor
    free(p);

    pthread_exit(NULL);
}
// Function to run the server and accept client connections
// It continuously accepts client connections, creates a thread for each client, and handles the client request.
// It uses the handle_client function to process the client's request.  
void run(int serverfd , nlist **mp){
     while(1){
         int clientfd = accept(serverfd , NULL , NULL);
         if(clientfd < 0){
              perror("Accept failed");
              continue;
         }
          printf("connected to client %d\n",clientfd);
        /* int *client_sock = (int*)malloc(sizeof(int));
         *client_sock = clientfd;
         pthread_t t1;
         pair *p1 = (pair*)malloc(sizeof(pair));
         p1->fd = client_sock;
         p1->mp = mp;
        if(pthread_create(&t1 , NULL , threaded_client_handler, (void*)p1) != 0){
            perror("thread creating failed !");
            close(clientfd);
            free(client_sock);
            free(p1);
        }
        pthread_detach(t1); // So we don't need to join it*/
       pair p1;
       p1.fd = &clientfd;
       p1.mp = mp;
       handle_client((void*)&p1);
       close(clientfd);
     }
     
     
}
