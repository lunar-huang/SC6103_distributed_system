#include <stdint.h>  // For uint8_t and uint32_t types
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#ifdef __linux__
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <fcntl.h> 
#elif _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#pragma comment(lib, "ws2_32.lib")  // Link Winsock library for Windows
#endif

#include <pthread.h>
#include "server.h"
#include "communication.h"  // Include marshalling and unmarshalling functionality
#include <mysql/mysql.h>
#include <fcntl.h>  // For setting non-blocking mode

#define PORT 8080  // Server port
#define BUFFER_SIZE 1024  // Buffer size for communication
#define MAX_HISTORY 100  // Maximum number of request histories to store
#define SERVER_IP "172.20.10.10"  // Server IP address

// Flight and related data initialization
Flight *flights = NULL;  // Initialize to NULL
int flight_count = 0;
int max_flights = 100;

// Mutex for thread-safe operations
pthread_mutex_t flight_mutex = PTHREAD_MUTEX_INITIALIZER;

// Structure for storing request history
typedef struct {
    struct sockaddr_in client_addr;  // Client address
    char request[BUFFER_SIZE];  // Original request
    char response[BUFFER_SIZE];  // Cached response
} RequestHistory;

RequestHistory history[MAX_HISTORY];  // History array to store requests and responses
int history_count = 0;  // Current count of stored requests
int use_at_least_once = 0;  // Flag to toggle between at-least-once and at-most-once modes

// Function to set a socket to non-blocking mode
void set_nonblocking(int sockfd) {
#ifdef _WIN32
    u_long mode = 1;  // Enable non-blocking mode on Windows
    ioctlsocket(sockfd, FIONBIO, &mode);
#else
    int flags = fcntl(sockfd, F_GETFL, 0);  // Get current socket flags
    if (flags == -1) {
        perror("fcntl F_GETFL failed");
        return;
    }
    if (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) == -1) {  // Set socket to non-blocking mode
        perror("fcntl F_SETFL failed");
    }
#endif
}

// Store a processed request and its response into the request history
void store_in_history(struct sockaddr_in *client_addr, const char *request, const char *response) {
    if (history_count < MAX_HISTORY) {
        history[history_count].client_addr = *client_addr;  // Copy client address
        strncpy(history[history_count].request, request, BUFFER_SIZE);  // Copy request
        strncpy(history[history_count].response, response, BUFFER_SIZE);  // Copy response
        history_count++;
    } else {
        // FIFO: Remove the oldest entry to make room for new one
        for (int i = 1; i < MAX_HISTORY; i++) {
            history[i - 1] = history[i];
        }
        history[MAX_HISTORY - 1].client_addr = *client_addr;
        strncpy(history[MAX_HISTORY - 1].request, request, BUFFER_SIZE);
        strncpy(history[MAX_HISTORY - 1].response, response, BUFFER_SIZE);
    }
}

// Check if a request has already been processed (to avoid duplicates)
int find_in_history(int sockfd, struct sockaddr_in *client_addr, const char *request, char *response) {
    for (int i = 0; i < history_count; i++) {
        if (strcmp(history[i].request, request) == 0 &&  // Check if request matches
            history[i].client_addr.sin_addr.s_addr == client_addr->sin_addr.s_addr &&  // Check client address
            history[i].client_addr.sin_port == client_addr->sin_port) {
            
            strcpy(response, history[i].response);  // Copy the cached response
            printf("Request duplicated! Returning cached response.\n");
            // Send the cached response to the client
            sendto(sockfd, response, strlen(response), 0, (struct sockaddr *)client_addr, sizeof(*client_addr));
            return 1;  // Request has already been processed
        }
    }
    printf("Request goes further for processing...\n");
    return 0;  // No duplicate found
}

// Thread function to handle client requests
void *handle_client(void *arg) {
    struct client_data *data = (struct client_data *)arg;  // Extract client data from the argument
    char reply[BUFFER_SIZE];

    MYSQL *conn = data->conn;  // Use the passed database connection

    printf("handle_client: processing request!\n");

    if (use_at_least_once) {
        // At-least-once: Directly re-execute the request
        printf("Processing new request (At-least-once): %u\n", data->buffer);
        handleRequest(data->buffer, data->client_addr, data->sockfd, data->addr_len, conn);

        // Generate a new response
        snprintf(reply, sizeof(reply), "Response to: %s", data->buffer);
    } else {
        // At-most-once: Check the history to avoid duplicate processing
        if (find_in_history(data->sockfd, &data->client_addr, data->buffer, reply)) {
            // Re-reply: Return the cached response for the duplicate request
            printf("Duplicate request found (At-most-once), sending cached response.\n");
            // Cached response has already been sent in find_in_history
        } else {
            // Process the new request
            printf("Processing new request (At-most-once): %s\n", data->buffer);
            handleRequest(data->buffer, data->client_addr, data->sockfd, data->addr_len, conn);

            // Generate a new response
            snprintf(reply, sizeof(reply), "Response to: %s", data->buffer);
            // Store the processed request and response in history
            store_in_history(&data->client_addr, data->buffer, reply);
        }
    }

    // Release dynamically allocated memory for client data
    free(data);

    return NULL;
}

// Main function to set up the server
int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s [at-least-once | at-most-once]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Parse the command-line argument to select fault-tolerance mode
    if (strcmp(argv[1], "at-least-once") == 0) {
        use_at_least_once = 1;
        printf("Running with at-least-once fault tolerance.\n");
    } else if (strcmp(argv[1], "at-most-once") == 0) {
        use_at_least_once = 0;
        printf("Running with at-most-once fault tolerance.\n");
    } else {
        printf("Invalid argument. Use 'at-least-once' or 'at-most-once'.\n");
        exit(EXIT_FAILURE);
    }

#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        perror("WSAStartup failed");
        exit(EXIT_FAILURE);
    }
#endif

    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    char *buffer = (char *)malloc(BUFFER_SIZE * sizeof(char));  // Allocate buffer memory
    if (buffer == NULL) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }
    socklen_t addr_len = sizeof(client_addr);

    // Create a UDP socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        free(buffer);
        exit(EXIT_FAILURE);
    }

    // Set the socket to non-blocking mode
    set_nonblocking(sockfd);

    // Configure the server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    server_addr.sin_port = htons(PORT);

    // Bind the socket to the specified IP and port
    if (bind(sockfd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(sockfd);
        free(buffer);
        exit(EXIT_FAILURE);
    }

    printf("Server is running on port %d...\n", PORT);

    // Connect to the database
    MYSQL *conn = connect_db();
    query_flights(conn);  // Query and display current flights

    printf("Successfully connected to the database!\n");

    // Main loop: continuously handle incoming client requests
    while (1) {
        memset(buffer, 0, BUFFER_SIZE);  // Clear the buffer

        // Use select to monitor socket readiness for reading
        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(sockfd, &read_fds);

        struct timeval timeout;
        timeout.tv_sec = 5;
        timeout.tv_usec = 0;

        int activity = select(sockfd + 1, &read_fds, NULL, NULL, &timeout);
        if (activity < 0) {
            perror("select error");
        } else if (activity == 0) {
            continue;  // Timeout without activity
        }

        // Receive a client request
        int n = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&client_addr, &addr_len);
        if (n < 0) {
            if (errno == EWOULDBLOCK || errno == EAGAIN) {
                printf("No data received yet.\n");
                continue;
            }
            perror("Receive failed");
            continue;
        }

        // Create a new thread to handle the client's request
        pthread_t client_thread;
        struct client_data *data = malloc(sizeof(struct client_data));  // Allocate memory for client data
        if (!data) {
            perror("Malloc failed");
            continue;
        }

        // Copy the received data and client information
        strncpy(data->buffer, buffer, BUFFER_SIZE);
        data->client_addr = client_addr;
        data->sockfd = sockfd;
        data->addr_len = addr_len;
        data->conn = conn;  // Pass the database connection to the thread

        // Create a new thread to handle the request
        if (pthread_create(&client_thread, NULL, handle_client, (void *)data) != 0) {
            perror("Client thread creation failed");
            free(data);  // Free memory if thread creation fails
        }
        pthread_detach(client_thread);  // Detach the thread so it cleans up after itself
    }

#ifdef _WIN32
    WSACleanup();
#endif

    close(sockfd);
    free(buffer);  // Free the buffer memory

    // Close the database connection
    close_db(conn);
    return 0;
}
