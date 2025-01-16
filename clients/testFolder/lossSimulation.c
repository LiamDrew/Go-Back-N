#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <errno.h>

#define DATA_PACKET_SIZE 512
#define MAX_FILENAME_LEN 20
#define MAX_LOSS_SEQ 50

// Packet structures
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

typedef struct {
    char type;
    char sequence_number;
} AckPacket;

void error_exit(const char *message) {
    perror(message);
    exit(1);
}

// Parse the list of sequence numbers to simulate ACK loss
int parse_loss_list(const char *arg, int *loss_list) {
    int count = 0;
    char *token;
    char *arg_copy = strdup(arg);

    token = strtok(arg_copy, ",");
    while (token != NULL && count < MAX_LOSS_SEQ) {
        loss_list[count++] = atoi(token);
        token = strtok(NULL, ",");
    }

    free(arg_copy);
    return count;
}

// Check if a sequence number should be lost (i.e., not acknowledged) and remove it if found
int is_lost(int seq, int *loss_list, int *loss_count) {
    for (int i = 0; i < *loss_count; i++) {
        if (loss_list[i] == seq) {
            // Shift elements to remove the sequence number from the list
            for (int j = i; j < *loss_count - 1; j++) {
                loss_list[j] = loss_list[j + 1];
            }
            (*loss_count)--;
            return 1;
        }
    }
    return 0;
}


int main(int argc, char *argv[]) {
    if (argc != 6) {
        fprintf(stderr, "Usage: %s <server_ip> <port> <filename> <window_size> <loss_list>\n", argv[0]);
        fprintf(stderr, "       loss_list is a comma-separated list of sequence numbers to simulate ACK loss\n");
        exit(1);
    }

    const char *server_ip = argv[1];
    int port = atoi(argv[2]);
    const char *filename = argv[3];
    int window_size = atoi(argv[4]);
    const char *loss_list_str = argv[5];

    if (window_size < 1 || window_size > 9) {
        fprintf(stderr, "Invalid window size. Must be between 1 and 9.\n");
        exit(1);
    }

    // Parse the loss list from the argument
    int loss_list[MAX_LOSS_SEQ];
    int loss_count = parse_loss_list(loss_list_str, loss_list);

    // Open the file for writing
    FILE *file = fopen(filename, "wb");
    if (!file) {
        perror("Failed to open file for writing");
        exit(1);
    }

    // Create UDP socket
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        fclose(file);
        error_exit("Socket creation failed");
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        fclose(file);
        error_exit("Invalid server IP address");
    }

    // Prepare RRQ packet
    RRQPacket rrq_packet;
    rrq_packet.type = 1;
    rrq_packet.window_size = window_size;
    strncpy(rrq_packet.filename, filename, MAX_FILENAME_LEN - 1);
    rrq_packet.filename[MAX_FILENAME_LEN - 1] = '\0';

    // Send RRQ packet to server
    if (sendto(sockfd, &rrq_packet, sizeof(rrq_packet), 0, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        fclose(file);
        error_exit("Failed to send RRQ packet");
    }

    printf("Sent RRQ for file %s with window size %d\n", filename, window_size);

    // Variables for receiving data and sending ACKs
    DataPacket data_packet;
    AckPacket ack_packet;
    struct sockaddr_in from_addr;
    socklen_t from_len = sizeof(from_addr);
    int expected_sequence = 0;

    while (1) {
        // Receive data packet
        ssize_t recv_len = recvfrom(sockfd, &data_packet, sizeof(data_packet), 0, (struct sockaddr *)&from_addr, &from_len);
        if (recv_len < 0) {
            fclose(file);
            error_exit("Error receiving data packet");
        }

        // Check if packet is DATA packet
        if (data_packet.type == 2) {  // DATA packet
            if (data_packet.sequence_number == expected_sequence) {
                // Calculate actual data size received (excluding type and sequence number bytes)
                size_t data_size = recv_len - 2;  // Header is 2 bytes

                // printf("\n\nLiam says data size is %ld\n\n", data_size);

                // Check if this is the last packet (EOF) based on data size
                if (data_size < DATA_PACKET_SIZE) {
                    printf("Received final DATA packet %d with %zd bytes (EOF)\n", data_packet.sequence_number, data_size);

                    // Check if this ACK should be "lost" (not sent)
                    if (is_lost(data_packet.sequence_number, loss_list, &loss_count)) {
                        printf("Not sending ACK and simulating loss of ACK %d\n", data_packet.sequence_number);
                        continue;
                    } else {
                        // Write last data to file if there is any data
                        if (data_size > 0) {
                            fwrite(data_packet.data, 1, data_size, file);
                        }

                        // Send ACK for the EOF packet
                        ack_packet.type = 3;  // ACK
                        ack_packet.sequence_number = data_packet.sequence_number;
                        if (sendto(sockfd, &ack_packet, sizeof(ack_packet), 0, (struct sockaddr *)&server_addr, from_len) < 0) {
                            fclose(file);
                            error_exit("Failed to send ACK for EOF");
                        }
                        printf("Sent ACK for EOF packet %d\n", ack_packet.sequence_number);
                    }

                    // End of file transfer, so break the loop
                    printf("\nFile transfer completed successfully.\n");
                    break;
                } else {
                    // Normal data packet (not EOF)
                    printf("Received DATA packet %d with %zd bytes\n", data_packet.sequence_number, data_size);

                    // Check if this ACK should be "lost" (not sent)
                    if (is_lost(data_packet.sequence_number, loss_list, &loss_count)) {
                        printf("Not sending ACK and simulating loss of ACK %d\n", data_packet.sequence_number);
                        continue;
                    } else {
                        fwrite(data_packet.data, 1, data_size, file);

                        // Prepare and send ACK for the received packet
                        ack_packet.type = 3;  // ACK
                        ack_packet.sequence_number = data_packet.sequence_number;
                        if (sendto(sockfd, &ack_packet, sizeof(ack_packet), 0, (struct sockaddr *)&server_addr, from_len) < 0) {
                            fclose(file);
                            error_exit("Failed to send ACK");
                        }
                        printf("Sent ACK %d\n", ack_packet.sequence_number);
                    }

                    // Move to the next expected sequence number
                    expected_sequence++;
                }
            } else {
                printf("Out-of-order packet received (seq %d), expected %d. Ignoring.\n", data_packet.sequence_number, expected_sequence);
            }
        } else if (data_packet.type == 4) {  // ERROR packet
            fprintf(stderr, "Received ERROR packet from server. Exiting.\n");
            fclose(file);
            break;
        }
    }

    fclose(file);
    close(sockfd);
    return 0;
}
