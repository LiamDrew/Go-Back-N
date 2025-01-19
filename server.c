/**
 * @file server.c
 * @author Liam Drew
 * @date November 2024
 * @brief
 * Implementation of a file transfer server. Listens for file requests from
 * clients and transfers requested files in 512 byte data packets. Implements
 * the Go-Back-N (Sliding Window) reliability protocol over UDP.
*/

#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>
#include "transfer.h"

#define BUFSIZE 1024
#define SRCDIR "source_files/"

typedef struct {
    u_int8_t type;
} msg_type;

typedef struct {
    u_int8_t type;
    u_int8_t window_size;
    char filename[20];
} rrq;

static void error(char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char **argv) {
    int sockfd;
    int portno;
    socklen_t clientlen;
    struct sockaddr_in serveraddr;
    struct sockaddr_in clientaddr;
    struct hostent *hostp;
    char *hostaddrp;
    int optval;
    char buf[BUFSIZE];

    /* check command line args */
    if (argc != 2) { 
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    portno = atoi(argv[1]);

    /* create the parent socket */
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) { error("ERROR opening socket"); }

    /* allow server to be rerun immediately */
    optval = 1;    
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR,
        (const void *)&optval, sizeof(int));

    /* build the server's internet address */
    bzero((char*) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons((unsigned short)portno);

    /* bind the parent socket to a port */
    if (bind(sockfd, (struct sockaddr *) &serveraddr,
             sizeof(serveraddr)) < 0) {
        error("ERROR on binding");
    }

    int n;
    int fd;
    bool client_connected = false;
    clientlen = sizeof(clientaddr);

    while (true) {
        /* wait for a client request to come in */
        memset(buf, 0, BUFSIZE);
        n = recvfrom(sockfd, buf, BUFSIZE, 0,
            (struct sockaddr *) &clientaddr, &clientlen);
        if (n < 0) { error("ERROR recieving client request"); }

        printf("Just received client message\n");

        /* determine who sent the datagram */
        hostp = gethostbyaddr((const char *)&clientaddr.sin_addr.s_addr,
                               sizeof(clientaddr.sin_addr.s_addr), AF_INET);
        if (hostp == NULL) { error("ERROR getting sender"); }
        
        hostaddrp = inet_ntoa(clientaddr.sin_addr);
        if (hostaddrp == NULL) { error("ERROR here idk"); }
        printf("Recieved datagram from %s (%s)\n", hostp->h_name, hostaddrp);
        printf("Server recieved %ld/%d bytes: %s\n", strlen(buf), n, buf);

        assert(n <= sizeof(rrq)); // Ensure clients don't exceed write limit

        msg_type head;
        memset(&head, 0, sizeof(msg_type));
        memcpy(&head, buf, sizeof(msg_type));

        // Case 1: Message is connection request
        if (head.type == 1) { 
            if (client_connected) { assert(false); } // one client at a time

            client_connected = true;
            rrq req;
            memset(&req, 0, sizeof(rrq));
            memcpy(&req, buf, n);

            char filepath[35] = SRCDIR;
            strncat(filepath, req.filename, 20);

            /* try to open filename provided by client */
            fd = open(filepath, O_RDONLY);
            if (fd == -1) {     // File doesn't exist, send error msg
                printf("File cannot be opened, sending error message\n\n");
                if (!send_error(sockfd, &clientaddr, clientlen))
                    error("Failed to send error to client");
            } 
            
            else {      // Opened file, sending to client
                if (!send_file(sockfd, fd, req.window_size, &clientaddr, 
                               clientlen))
                    printf("Failed to send file to client\n");
            }
        } 
        
        // Case 2: Unexpected message type
        else {
            error("Server recieved an unexpected message type\n");
        }

        client_connected = false;  
    }
}