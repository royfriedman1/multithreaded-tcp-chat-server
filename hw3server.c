#include "hw3.h"

// Structure to define a connected client
typedef struct {
    struct sockaddr_in address; // Client's IP address structure
    int sockfd;                 // Socket descriptor
    int uid;                    // Unique ID (optional, but good for management)
    char name[32];              // Client name
    int is_active;              // Flag to mark if the slot is in use
} client_t;

// Global array of clients
client_t *clients[MAX_CLIENTS];
// Mutex to protect the clients array
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

// --- Helper Functions ---

// Add client to the global queue
void queue_add(client_t *cl) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (!clients[i]) {
            clients[i] = cl;
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

// Remove client from the global queue
void queue_remove(int uid) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (clients[i]) {
            if (clients[i]->uid == uid) {
                clients[i] = NULL;
                break;
            }
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

// Send message to all clients (Broadcast)
void send_message_all(char *s, int uid) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (clients[i]) {
            // Write to the socket
            if (write(clients[i]->sockfd, s, strlen(s)) < 0) {
                perror("ERROR: write to descriptor failed");
            }
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

// Send message to a specific client (Whisper)
// Returns 1 if found, 0 if not found
int send_message_private(char *s, char *target_name) {
    int found = 0;
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (clients[i]) {
            if (strcmp(clients[i]->name, target_name) == 0) {
                if (write(clients[i]->sockfd, s, strlen(s)) < 0) {
                    perror("ERROR: write to descriptor failed");
                }
                found = 1;
                break;
            }
        }
    }
    pthread_mutex_unlock(&clients_mutex);
    return found;
}

// Trim newline character (\n) from strings
void str_trim_lf(char* arr, int length) {
    for (int i = 0; i < length; i++) {
        if (arr[i] == '\n') {
            arr[i] = '\0';
            break;
        }
    }
}

// --- Thread Function ---

void *handle_client(void *arg) {
    // Variable 'buff_out' removed to avoid unused variable warning
    char buff_in[BUFFER_SIZE];
    int read_len;

    client_t *cli = (client_t *)arg;

    // 1. Receive name from client (First packet)
    if (recv(cli->sockfd, cli->name, 32, 0) <= 0 || strlen(cli->name) < 1) {
        printf("Didn't enter the name.\n");
        free(cli);
        return NULL;
    }
    
    // Ensure name is null terminated and trimmed
    str_trim_lf(cli->name, strlen(cli->name));

    // Print connection message on server console
    // Format: "client name connected from address"
    printf("client %s connected from %s\n", cli->name, inet_ntoa(cli->address.sin_addr));

    // Add to global list
    queue_add(cli);

    // 2. Main Chat Loop
    while (1) {
        bzero(buff_in, BUFFER_SIZE);
        read_len = recv(cli->sockfd, buff_in, BUFFER_SIZE, 0);

        if (read_len > 0) {
            str_trim_lf(buff_in, read_len);

            // Format the output message: "sourcename: message"
            // We use a temporary buffer for formatting
            char formatted_msg[BUFFER_SIZE + 50];
            bzero(formatted_msg, sizeof(formatted_msg));
            
            // Check for whisper ("@friend msg")
            if (buff_in[0] == '@') {
                // Parse target name
                char *target_name = strtok(buff_in + 1, " "); // Skip '@'
                char *msg_content = strtok(NULL, ""); // Get the rest of the string
                
                if (target_name && msg_content) {
                    sprintf(formatted_msg, "%s: @%s %s\n", cli->name, target_name, msg_content);
                    send_message_private(formatted_msg, target_name);
                }
            } else {
                // Normal broadcast
                sprintf(formatted_msg, "%s: %s\n", cli->name, buff_in);
                send_message_all(formatted_msg, cli->uid);
            }

        } else {
            // Client disconnected or error
            break;
        }
    }

    // 3. Cleanup
    close(cli->sockfd);
    
    // Print disconnection message
    // Format: "client name disconnected"
    printf("client %s disconnected\n", cli->name);
    
    queue_remove(cli->uid);
    free(cli);
    pthread_detach(pthread_self());

    return NULL;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        printf("Usage: %s <port>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int port = atoi(argv[1]);
    int listenfd = 0, connfd = 0;
    struct sockaddr_in serv_addr;
    struct sockaddr_in cli_addr;
    pthread_t tid;

    // Socket settings
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port);

    // Ignore pipe signals (prevents crash when client closes abruptly)
    signal(SIGPIPE, SIG_IGN);

    if (bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("ERROR: Socket binding failed");
        return EXIT_FAILURE;
    }

    if (listen(listenfd, 10) < 0) {
        perror("ERROR: Socket listening failed");
        return EXIT_FAILURE;
    }

    // Unique ID counter
    int uid = 10;

    while (1) {
        socklen_t clilen = sizeof(cli_addr);
        connfd = accept(listenfd, (struct sockaddr*)&cli_addr, &clilen);

        // Check for max clients
        // (Simplified: In a robust app we would check count before malloc)
        
        client_t *cli = (client_t *)malloc(sizeof(client_t));
        cli->address = cli_addr;
        cli->sockfd = connfd;
        cli->uid = uid++;

        // Create thread
        pthread_create(&tid, NULL, &handle_client, (void*)cli);

        // Reduce CPU usage slightly
        sleep(0); 
    }

    return EXIT_SUCCESS;
}

