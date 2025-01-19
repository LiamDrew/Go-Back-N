/**
 * @file transfer.c
 * @author Liam Drew
 * @date November 2024
 * @brief
 * Implementation of the transfer module. Manages the sliding window for the
 * file transfer.
*/

#include "transfer.h"
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>


#define DATASIZE 512
#define ERRSIZE sizeof(error_msg)
#define MSGSIZE sizeof(data)
#define DMCODE 2
#define DHSIZE sizeof(data_header)
#define TIMEOUT ((struct timeval){.tv_sec = TIMEOUT_S, .tv_usec = TIMEOUT_US})
#define TIMEOUT_S 3
#define TIMEOUT_US 0

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
} data_header;

typedef struct {
    u_int8_t type;
    u_int8_t seq_num;
} ack;

typedef struct {
    int seq_num;
    char *data;
    unsigned size;
    bool sent;
} packet;


static void error(char *msg)
{
    perror(msg);
    exit(1);
}

// Debugging utilities
void print_data(char buf[], int size)
{
    data_header dh;
    memcpy(&dh, buf, DHSIZE);

    printf("\nPrinting the buffer\n");
    printf("Message type: %d\n", dh.type);
    printf("SENDING packet with sequence number: %d\n", dh.seq_num);
    printf("Trying to send %d bytes\n", size);

    // Printing non-string contents
    for (int offset = 0; offset < size; offset++) {
        if (buf[offset] == '\0')
            putchar('.');
        else
            putchar(buf[offset]);
    }

    printf("\n");
}

void print_window(packet window[], int size)
{
    for (int i = 0; i < size; i++) {
        printf("Seq num is: %d\n", window[i].seq_num);

        // Printing the potentially non-string data
        for (int offset = 0; offset < window[i].size; offset++) {
            if (window[i].data[offset] == '\0') {
                putchar('.');
            }

            else {
                putchar(window[i].data[offset]);
            }
        }

        printf("\n");
    }
}

bool send_error(int sockfd, struct sockaddr_in *clientaddr, int clientlen)
{
    error_msg e;
    e.type = 4;

    char buf[1];
    memset(buf, 0, ERRSIZE);
    memcpy(buf, &e, ERRSIZE);

    int n = sendto(sockfd, buf, ERRSIZE, 0,
        (struct sockaddr *) clientaddr, clientlen);
    
    if (n < 0)
        return false;
    return true;
}


bool send_file(int sockfd, int filefd, const int set_window_size, 
               struct sockaddr_in *clientaddr, socklen_t clientlen)
{
    /* set up the sending window */
    const int window_size = set_window_size;
    packet window[window_size];
    packet p;
    for (int i = 0; i < window_size; i++) {
        p.seq_num = 0;
        p.data = NULL;
        p.size = -1;
        p.sent = false;
        memcpy(&window[i], &p, sizeof(packet));
    }

    /* declare vars for socket reading */
    int n;
    fd_set active_fd_set, read_fd_set;
    FD_ZERO(&active_fd_set);
    FD_SET(sockfd, &active_fd_set);
    
    /* establish buffers*/
    char data_buf[DATASIZE];
    char msg_buf[MSGSIZE];

    /* establish seq numbers */
    int cum_sequence_num = 0;
    int cum_ack_num = -1;

    /* track the number of consecutive timeouts */
    int timeouts = 0;

    bool finished_sending = false;
    bool reception_confirmed = false;
    while (!finished_sending || !reception_confirmed) {
        
        // Step 1: Prepare the sending window
        if (!finished_sending) {
            for (int i = 0; i < window_size; i++) {
                // If the window slot is empty, fill it with more file data
                if (window[i].data == NULL) {
                    memset(data_buf, 0, DATASIZE);
                    n = read(filefd, data_buf, DATASIZE);

                    if (n < 0) { error("ERROR: Reached EOF early"); }

                    char *fcontent = calloc(n, sizeof(char));
                    memcpy(fcontent, data_buf, n);

                    window[i].seq_num = cum_sequence_num; 
                    window[i].data = fcontent;
                    window[i].size = n;
                    window[i].sent = false;

                    if (n < DATASIZE) {    // Just reached EOF
                        finished_sending = true;
                        break;
                    }

                    cum_sequence_num++;
                }
            }
        }    

        // Step 2: Send the packets
        for (int i = 0; i < window_size; i++) {
            // If there's data to send and it hasn't been sent already, send it
            if (window[i].data != NULL && !window[i].sent) {
                msg_buf[0] = DMCODE;
                msg_buf[1] = window[i].seq_num;
                memcpy(msg_buf + DHSIZE, window[i].data, window[i].size);

                // print_data(msg_buf, window[i].size + DHSIZE);

                n = sendto(sockfd, msg_buf, window[i].size + DHSIZE, 0, 
                    (struct sockaddr *) clientaddr, clientlen);
                if (n < 0) { error("Error sending packet\n"); }
                
                window[i].sent = true;
            }
        }

        // Step 3: Listen for acks

        int ack_index = -1;
        read_fd_set = active_fd_set;

        ack a;
        memset(&a, 0, sizeof(ack));
        char ack_buf[sizeof(ack)];

        int read = select(FD_SETSIZE, &read_fd_set, NULL, NULL, &TIMEOUT);
        if (read < 0) { 
            error("Error with select"); 
        } 
        
        else if (read == 0) {
            printf("Timeout occurred\n");
            timeouts++;

            if (timeouts >= 5) {
                // After 5 timeouts, give up on sending to the client
                return false;
            }
            for (int i = 0; i < window_size; i++) {
                window[i].sent = false;
            }

        }

        else {
            if (FD_ISSET(sockfd, &read_fd_set)) {
                //try to read an ACK from sockfd
                memset(ack_buf, 0, sizeof(ack));
                n = recvfrom(sockfd, ack_buf, sizeof(ack), 0,
                    (struct sockaddr *) clientaddr, &clientlen);
                if (n < 0) { error("Failed to read ACK"); }

                memcpy(&a, ack_buf, sizeof(ack));

                bool ack_found = false;

                for (int i = window_size - 1; i >= 0; i--) {
                    if (a.seq_num == window[i].seq_num) {
                        ack_found = true;
                        ack_index = i;
                    }

                    if (ack_found) {
                        if (window[i].data != NULL) {
                            free(window[i].data);
                            window[i].data = NULL;
                        }
                        window[i].size = 0;
                        window[i].sent = false;

                        timeouts = 0;

                    }
                }
                
                if (a.seq_num > cum_ack_num) {
                    cum_ack_num = a.seq_num;
                }
            }
        }



        // Step 4: Adjust window based on ACKs
        /* only bother adjust the window if more packets need to be sent */
        if (!finished_sending) {

            if (ack_index != -1) {
                int shift_count = ack_index + 1;

                for (int i = 0; i < window_size - shift_count; i++) {
                    window[i] = window[i + shift_count];
                }

                for (int i = window_size - shift_count; i < window_size; i++) {
                    window[i].data = NULL;
                    window[i].sent = false;
                    window[i].size = 0;
                    window[i].seq_num = -1;
                }
            }
        }
        
        // Step 5: Confirm that all ACKs have been received
        if (finished_sending) {
            if (cum_sequence_num == cum_ack_num) {
                reception_confirmed = true;
            }
        }
    }

    printf("File transfer complete.\n");
    return true;
}
