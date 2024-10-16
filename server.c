#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#define PORT 80

int openHTML(int *acceptfd);

int main(int argc, char *argv[]){

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
  int listenfd = listen(socketfd, 0);
  if (listenfd < 0){
    perror("listen failed");
    exit(EXIT_FAILURE);
  }

  // accept()
  socklen_t addr_size = sizeof address;
  int acceptfd = accept(socketfd, (struct sockaddr*)&address, &addr_size);
  if (acceptfd < 0){
    perror("accept failed");
    exit(EXIT_FAILURE);
  } 

  /*
    Request GET
    Response 200 OK
  */
  char *request = "GET / HTTP/1.1";
  /*
  char *connected = "Server Connected!"; 
  int len, bytes_sent;
  len = strlen(connected);
  bytes_sent = send(socketfd, connected, len, 0);
  */
  // recv() data and print it out
  char buffer[1024] = {0};
  int len = 1024;
  ssize_t bytes_recv = recv(acceptfd, buffer, len, 0); 

  if (bytes_recv < 0){
    perror("recv failed");
  }
  // finding the pattern
  char *msg_path1 = "HTTP/1.1 200 OK\r\n\r\n";
  if (strstr(buffer, request)){
    ssize_t bytes_send = send(acceptfd, msg_path1, strlen(msg_path1), 0);
  } 
  // TODO: Implement correct requests
  // debug
  printf("%s", buffer);
  printf("%s", msg_path1);

  /*
    www/index.html
  */
  openHTML(&acceptfd);


  // close() 
  shutdown(socketfd, 0); 
  return EXIT_SUCCESS;

}

// open html read, and send it
int openHTML(int *acceptfd){
  char *buffer;
  long file_length = 0;
  FILE *fp = fopen("www/index.html", "rb");
  if (fp == NULL){
    perror("Error opening file");
    return 1;
  }
  fseek(fp, 0, SEEK_END);
  file_length = ftell(fp);
  rewind(fp);
  buffer = (char *) malloc(file_length * sizeof(char));
  fread(buffer, file_length, 1, fp);
  if (send(*acceptfd, buffer, file_length, 0)){
    perror("send failed");
    exit(EXIT_FAILURE);
  }

  printf("count of bytes in file: %ld\n", file_length);
  //char storage[file_length];
  fclose(fp);
  return 0;
}









