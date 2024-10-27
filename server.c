#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>


// PORT USED
#define PORT 80

// Requests that are valid
#define REQUEST_VALID1 "GET / HTTP/1.1"
#define REQUEST_VALID2 "GET /index.html HTTP/1.1"

// Responses
//#define RESPONSEPATH "HTTP/1.1 200 OK\r\n\r\nRequested path: <the path>\r\n"
#define RESPONSE200 "HTTP/1.1 200 OK\r\n\r\n"
#define RESPONSE400 "HTTP/1.1 400 Not Found\r\n\r\n"

// Function declarations
int openHTML(int *acceptfd);
int response_http(int *acceptfd, char *buffer);
void* handle_client(void* arg);

int main(int argc, char *argv[]){

 
 
  // TODO: REFACTOR -> move server logic back into main. And implement threads
  // create socket()
  int socketfd = socket(AF_INET , SOCK_STREAM, 0);
  if (socketfd < 0){
    perror("socket failed");
    exit(EXIT_FAILURE);
  }

  // setup
  // set address port 80 
  int opt = 1;
  int setsockopt_fd = setsockopt(socketfd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));
  if (setsockopt_fd < 0){
    perror("setsockopt failed");
    exit(EXIT_FAILURE);
  }
  // address format
  struct sockaddr_in address;  
  memset(&address, 0, sizeof address);
  address.sin_family = AF_INET;
  address.sin_port = htons(PORT);
  inet_pton(AF_INET, "127.0.0.1", &address.sin_addr);
  
  // bind()
  int bindfd = bind(socketfd, (struct sockaddr*)&address, sizeof address); 
  if (bindfd < 0){
    perror("bind failed");
    exit(EXIT_FAILURE);
  }

  // listen()
  int listenfd = listen(socketfd, 5);
  if (listenfd < 0){
    perror("listen failed");
    exit(EXIT_FAILURE);
  }

  printf("server listening on port %d\n", PORT);

  while(1){
    // accept()
    socklen_t addr_size = sizeof address;
    int acceptfd = accept(socketfd, (struct sockaddr*)&address, &addr_size);
    if (acceptfd < 0){
      perror("accept failed");
      exit(EXIT_FAILURE);
    } 

    // create thread to handle client
   // allocate for threads
    pthread_t thread_id;
    int rc;
    int *sock_ptr = malloc(sizeof(int));
    *sock_ptr = acceptfd;
    // create thread
    rc = pthread_create(&thread_id, NULL, handle_client, (void*) sock_ptr);
    if (rc < 0){
      perror("pthread_create failed");
      exit(EXIT_FAILURE);
    }
    // reclaim thread when done (not overloading)
    pthread_detach(thread_id);

     
  }
  shutdown(socketfd, 0); 
  return EXIT_SUCCESS;

}
// client handling process
void *handle_client(void *arg){
   // allocate for requests buffer
    char buffer[1024] = {0};
    int len = 1024;
    int acceptfd = * (int*) arg;
    // recv request
    ssize_t bytes_recv = recv(acceptfd, buffer, len, 0); 
    printf("bytes_recv: %ld\n", bytes_recv);
    if (bytes_recv < 0){
      perror("recv error:");
      exit(EXIT_FAILURE);
    }
    printf("%s\n", buffer);
   
   // send response 
    int response = response_http(&acceptfd, buffer);
    printf("response: %i\n", response);
    if (response == 0){
    // send html
      openHTML(&acceptfd);
    } else {
      perror("response 400");
      exit(EXIT_FAILURE);
    } 
    
    // close client
    shutdown(acceptfd, 0);
    return NULL;
}

/* find the pattern in request ; returns 0 if success, 1 if response is 400*/
int response_http(int *acceptfd, char *buffer){
  ssize_t bytes_send;
  if (strstr(buffer, REQUEST_VALID1) || strstr(buffer, REQUEST_VALID2) ){
    bytes_send = send(*acceptfd, RESPONSE200, strlen(RESPONSE200), 0);
    printf("%s", RESPONSE200);
    return EXIT_SUCCESS;
  } else { 
    bytes_send = send(*acceptfd, RESPONSE400, strlen(RESPONSE400), 0); 
    printf("%s", RESPONSE400);
    return EXIT_FAILURE;
  }
}

// open html read, and send it
int openHTML(int *acceptfd){
  sleep(20);
// allocate for buffer
  char *buffer;
  long file_length = 0;
  // open file
  FILE *fp = fopen("www/index.html", "rb");
  printf("Path: index.html\n");
  // err check
  if (fp == NULL){
    perror("Error opening file");
    return EXIT_FAILURE;
  }
  // get the length of the file
  fseek(fp, 0, SEEK_END); //go forward to the end
  file_length = ftell(fp); // get length
  rewind(fp); // go back to the beginning
  // allocate for buffer
  buffer = (char *) malloc(file_length * sizeof(char));
  // read file
  fread(buffer, file_length, 1, fp);
  // err check
  if (!(send(*acceptfd, buffer, file_length, 0))){
    perror("send failed");
    return EXIT_FAILURE; 
  }
  // bytes in file
  printf("count of bytes in file: %ld\n", file_length);
  //char storage[file_length];
  // free and exits
  free(buffer);
  fclose(fp);
  return EXIT_SUCCESS;
}

