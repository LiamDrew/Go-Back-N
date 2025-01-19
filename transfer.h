/**
 * @file transfer.h
 * @author Liam Drew
 * @date November 2024
 * @brief
 * Defines the interface for the transfer module. If the client request is
 * valid, the server will send a valid response, and will send an error message
 * otherwise.
*/

#include <stdbool.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <arpa/inet.h>

bool send_error(int sockfd, struct sockaddr_in *clientaddr, int clientlen);

bool send_file(int sockfd, int filefd, const int set_window_size, 
               struct sockaddr_in *clientaddr, socklen_t clientlen);