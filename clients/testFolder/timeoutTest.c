#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <errno.h>

#define DATA_PACKET_SIZE 512
#define MAX_FILENAME_LEN 20
#define MAX_RETRIES 5
#define WAIT_TIME 4  // Seconds to wait after max retries

// Packet structures to match server's protocol
typedef struct {
    char type;
    char window_size;
    char filename[MAX_FILENAME_LEN];
} RRQPacket;

typedef struct {
    char type;
    char sequence_number;
    char data[DATA_PACKET_SIZE];
} DataPacket;

void error_exit(const char *message) {
    perror(message);
    exit(1);
}

int main(int argc, char *argv[]) {
    if (argc != 5) {
        fprintf(stderr, "Usage: %s <server_ip> <port> <filename> <window_size>\n", argv[0]);
        exit(1);
    }

    const char *server_ip = argv[1];
    int port = atoi(argv[2]);
    const char *filename = argv[3];
    int window_size = atoi(argv[4]);

    if (window_size < 1 || window_size > 9) {
        fprintf(stderr, "Invalid window size. Must be between 1 and 9.\n");
        exit(1);
    }

    // Create UDP socket
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        error_exit("Socket creation failed");
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        error_exit("Invalid server IP address");
    }

    // Prepare RRQ packet
    RRQPacket rrq_packet;
    rrq_packet.type = 1;  // RRQ
    rrq_packet.window_size = window_size;
    strncpy(rrq_packet.filename, filename, MAX_FILENAME_LEN - 1);
    rrq_packet.filename[MAX_FILENAME_LEN - 1] = '\0';

    // Send RRQ packet to server
    if (sendto(sockfd, &rrq_packet, sizeof(rrq_packet), 0, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        error_exit("Failed to send RRQ packet");
    }

    printf("Sent RRQ for file %s with window size %d\n", filename, window_size);

    // Variables for receiving data and tracking retries
    DataPacket data_packet;
    struct sockaddr_in from_addr;
    socklen_t from_len = sizeof(from_addr);
    // Track retries by sequence number
    int retries[256] = {0};
    int max_retries_reached = 0;

    while (1) {
        // Receive data packet with a 3-second timeout
        struct timeval timeout;
        timeout.tv_sec = 3;
        timeout.tv_usec = 0;
        setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

        ssize_t recv_len = recvfrom(sockfd, &data_packet, sizeof(data_packet), 0, (struct sockaddr *)&from_addr, &from_len);

        if (recv_len < 0) {
            if (errno == EWOULDBLOCK || errno == EAGAIN) {
                if (max_retries_reached) {
                    // Wait an additional 4 seconds to confirm no further retransmissions
                    printf("Waiting %d seconds to confirm no further retransmissions from server...\n", WAIT_TIME);
                    timeout.tv_sec = WAIT_TIME;
                    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

                    recv_len = recvfrom(sockfd, &data_packet, sizeof(data_packet), 0, (struct sockaddr *)&from_addr, &from_len);
                    if (recv_len < 0 && (errno == EWOULDBLOCK || errno == EAGAIN)) {
                        printf("No further retransmissions from server. Server has stopped after %d retries.\n", MAX_RETRIES);
                    } else if (recv_len > 0) {
                        printf("Unexpected packet received after max retries: Sequence %d\n", data_packet.sequence_number);
                    }
                    break;
                }
            } else {
                error_exit("Error receiving data packet");
            }
        } else if (data_packet.type == 2) {  // DATA packet
            int sequence_number = data_packet.sequence_number;
            printf("Received DATA packet %d\n", sequence_number);

            // Track retries for this sequence number
            retries[sequence_number]++;

            // Check if the same packet has been received 5 times
            if (retries[sequence_number] >= MAX_RETRIES) {
                printf("Packet %d was received %d times without an ACK. Server should stop retrying.\n",
                       sequence_number, retries[sequence_number]);
                max_retries_reached = 1;
            }
        } else if (data_packet.type == 4) {  // ERROR packet
            fprintf(stderr, "Received ERROR packet from server. Exiting.\n");
            break;
        }
    }

    close(sockfd);
    printf("Test completed. Server behavior observed for timeout and retransmission.\n");
    return 0;
}
