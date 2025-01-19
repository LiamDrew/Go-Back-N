/**
 * @file client.c
 * @author Liam Drew
 * @brief
 * A simple UDP client
 * usage: udpclient <host> <port>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdbool.h>
#include <assert.h>
#include <fcntl.h>

#define BUFSIZE 1024
#define DATASIZE 512


typedef struct {
    u_int8_t type;
    u_int8_t window_size;
    char filename[20];
} rrq;

typedef struct {
    u_int8_t type;
} msg_type;


typedef struct {
    u_int8_t type;
} error_msg;

typedef struct {
    u_int8_t type;
    u_int8_t seq_num;
    char data[DATASIZE];
} data;

typedef struct {
    u_int8_t type;
    u_int8_t seq_num;
} ack;

#define data_head ack

void error(char *msg) {
    perror(msg);
    exit(0);
}


void print_data(char buf[], int size)
{
    // Printing the potentially non-string data
    for (int offset = 0; offset < size; offset++) {
        if (buf[offset] == '\0') {
            putchar('.');
        }

        else {
            putchar(buf[offset]);
        }
    }

    printf("\n");
}


int main(int argc, char **argv) {
    int sockfd, portno, n;
    int serverlen;
    struct sockaddr_in serveraddr;
    struct hostent *server;
    char buf[BUFSIZE];
    char *hostname;

    /* check command line arguments */
    if (argc != 3)
    {
        fprintf(stderr, "usage: %s <host> <port>\n", argv[0]);
        exit(0);
    }

    hostname = argv[1];
    portno = atoi(argv[2]);

    /* socket: create the socket */
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");

    /* gethostbyname: get the server's DNS entry */
    server = gethostbyname(hostname);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host as %s\n", hostname);
        exit(0);
    }

    /* build the server's Internet address */
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
	  (char *)&serveraddr.sin_addr.s_addr, server->h_length);
    serveraddr.sin_port = htons(portno);


    /* send the rrq to the server */
    rrq r;
    r.type = 1;
    r.window_size = 1;
    strcpy(r.filename, "liam.txt");

    bzero(buf, BUFSIZE);
    memcpy(buf, &r, sizeof(rrq));

    /* send the message to the server */
    serverlen = sizeof(serveraddr);
    n = sendto(sockfd, buf, sizeof(rrq), 0, 
        (struct sockaddr *) &serveraddr, serverlen);
    if (n < 0)
      error("ERROR in sendto");
    
    bzero(buf, BUFSIZE);
    
    /* receive reply from server*/
    bool done_recieving = false;
    ack a;
    char ack_buf[sizeof(ack)];

    int fd = open("copy_liam.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    int bytes_written = 0;

    if (fd == -1) {
        error("Error opening file\n");
    }

    while (!done_recieving) {
        printf("Trying to receive a packet\n");

        // Step 1: Recieve a packet
        n = recvfrom(sockfd, buf, 514, 0, 
            (struct sockaddr *) &serveraddr, (socklen_t *) &serverlen);
        if (n < 0) 
            error("ERROR in recvfrom");

        assert(n >= 2);

        memcpy(&a, buf, sizeof(ack));

        printf("Read in %d bytes\n", n);
        printf("Seq number is: %d\n", a.seq_num);
        print_data(buf + 2, n - 2);
        int x = write(fd, buf + 2, n - 2);

        if (n < 514) {
            done_recieving = true;
        }

        // Step 2: Send an Ack
        ack curr_ack;
        curr_ack.type = 3;
        curr_ack.seq_num = a.seq_num;

        memset(ack_buf, 0, sizeof(ack_buf));
        memcpy(ack_buf, &curr_ack, sizeof(ack));

        n = sendto(sockfd, ack_buf, sizeof(ack), 0,
            (struct sockaddr *) &serveraddr, serverlen);
        if (n < 0) 
            error("ERROR in sendto");

    }

    return 0;
}