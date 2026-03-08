#include "hw3.h"

// Flag to stop the receive thread when exiting
volatile int flag = 0;
int sockfd = 0;

void str_trim_lf(char* arr, int length) {
    for (int i = 0; i < length; i++) {
        if (arr[i] == '\n') {
            arr[i] = '\0';
            break;
        }
    }
}

// Thread to receive messages from server
void *recv_msg_handler(void *arg) {
    char message[BUFFER_SIZE] = {};
    while (1) {
        int receive = recv(sockfd, message, BUFFER_SIZE, 0);
        if (receive > 0) {
            printf("%s", message);
            // Clear buffer
            memset(message, 0, sizeof(message));
        } else if (receive == 0) {
            // Server disconnected
            break;
        } else {
            // Error or exit
            break;
        }
        
        if (flag) {
            break;
        }
    }
    return NULL;
}

int main(int argc, char **argv) {
    if (argc != 4) {
        printf("Usage: %s <addr> <port> <name>\n", argv[0]);
        return EXIT_FAILURE;
    }

    char *ip = argv[1];
    int port = atoi(argv[2]);
    char *name = argv[3];

    struct sockaddr_in serv_addr;

    // Create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    
    // Convert IP string to binary
    if (inet_pton(AF_INET, ip, &serv_addr.sin_addr) <= 0) {
        printf("Invalid address/ Address not supported \n");
        return -1;
    }

    // Connect
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("ERROR: Connection failed\n");
        return -1;
    }

    // Send name to server immediately
    send(sockfd, name, 32, 0);

    // Create receive thread
    pthread_t recv_msg_thread;
    if (pthread_create(&recv_msg_thread, NULL, (void *) recv_msg_handler, NULL) != 0) {
        printf("ERROR: pthread\n");
        return EXIT_FAILURE;
    }

    // Main Loop: Send messages
    char buffer[BUFFER_SIZE] = {};
    while (1) {
        // Read input from user
        fgets(buffer, BUFFER_SIZE, stdin);
        str_trim_lf(buffer, BUFFER_SIZE);

        if (strcmp(buffer, "!exit") == 0) {
            // Send the exit command to server so it broadcasts it
            send(sockfd, buffer, strlen(buffer), 0);
            flag = 1; // Signal thread to stop
            printf("client exiting\n");
            break;
        } else {
            // Send normal message or whisper
            send(sockfd, buffer, strlen(buffer), 0);
        }
        
        bzero(buffer, BUFFER_SIZE);
    }

    close(sockfd);
    return EXIT_SUCCESS;
}