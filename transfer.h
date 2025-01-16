#include <stdbool.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <arpa/inet.h>


void error(char *msg);

bool send_error(int sockfd, struct sockaddr_in *clientaddr, int clientlen);

bool send_file(int sockfd, int filefd, const int set_window_size, 
               struct sockaddr_in *clientaddr, socklen_t clientlen);