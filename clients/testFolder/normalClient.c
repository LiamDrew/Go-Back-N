#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <errno.h>

#define DATA_PACKET_SIZE 512
#define MAX_FILENAME_LEN 20

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
    int errorReceived = 0;

    while (1) {
        // Receive data packet
        ssize_t recv_len = recvfrom(sockfd, &data_packet, sizeof(data_packet), 0, (struct sockaddr *)&from_addr, &from_len);
        if (recv_len < 0) {
            fclose(file);
            error_exit("Error receiving data packet");
        }

        // Check packet type
        if (data_packet.type == 2) {  // DATA packet
            if (data_packet.sequence_number == expected_sequence) {
                // Calculate actual data size received (excluding type and sequence number bytes)
                size_t data_size = recv_len - 2;  // Header is 2 bytes

                // EOF condition: data_size < DATA_PACKET_SIZE (last packet)
                if (data_size < DATA_PACKET_SIZE) {
                    if (data_size > 0) {
                        // Write last data to file if there is any
                        fwrite(data_packet.data, 1, data_size, file);
                    }

                    printf("Received final DATA packet %d with %zd bytes (EOF)\n", data_packet.sequence_number, data_size);

                    // Send ACK for the final packet (EOF)
                    ack_packet.type = 3;  // ACK
                    ack_packet.sequence_number = data_packet.sequence_number;
                    if (sendto(sockfd, &ack_packet, sizeof(ack_packet), 0, (struct sockaddr *)&server_addr, from_len) < 0) {
                        fclose(file);
                        error_exit("Failed to send ACK for final packet (EOF)");
                    }
                    printf("Sent ACK for EOF packet %d\n", ack_packet.sequence_number);

                    printf("\nFile transfer completed successfully.\n");
                    break;  // Exit the loop after EOF
                } else {
                    // Normal data packet (not EOF)
                    fwrite(data_packet.data, 1, data_size, file);

                    printf("Received DATA packet %d with %zd bytes\n", data_packet.sequence_number, data_size);

                    // Prepare and send ACK for the received packet
                    ack_packet.type = 3;  // ACK
                    ack_packet.sequence_number = data_packet.sequence_number;
                    if (sendto(sockfd, &ack_packet, sizeof(ack_packet), 0, (struct sockaddr *)&server_addr, from_len) < 0) {
                        fclose(file);
                        error_exit("Failed to send ACK");
                    }
                    printf("Sent ACK %d\n", ack_packet.sequence_number);

                    // Move to the next expected sequence number
                    expected_sequence++;
                }
            } else {
                printf("Out-of-order packet received (seq %d), expected %d. Ignoring.\n", data_packet.sequence_number, expected_sequence);
            }
        } else if (data_packet.type == 4) {  // ERROR packet
            fprintf(stderr, "Received ERROR packet from server. Exiting.\n");
            errorReceived = 1;
            break;
        }
    }

    fclose(file);
    if (errorReceived) {
        // Delete the partially downloaded file
        if (remove(filename) == 0) {
            printf("Partially downloaded file '%s' deleted.\n", filename);
        } else {
            perror("Error deleting file");
        }

    }
    close(sockfd);
    return 0;
}
