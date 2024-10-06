#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define PORT 8080

int main(int argc, char *argv[]){

  // create socket()
  int socketfd = socket(AF_INET , SOCK_STREAM, 0);
  if (socketfd < 0){
    perror("socket failed");
    exit(EXIT_FAILURE);
  }

  // setup
  // set address port 8080 
  int opt = 1;
  int setsockopt_fd = setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));
  if (setsockopt_fd < 0){
    perror("setsockopt failed");
    exit(EXIT_FAILURE);
  }
  // address format
  struct sockaddr_in address;  
  address.sin_family = AF_INET;
  address.sin_port = htons(PORT);
  inet_pton(AF_INET, "10.0.2.15", &address.sin_addr);
  
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
  
  return EXIT_SUCCESS;

}
