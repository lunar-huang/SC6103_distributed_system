#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>  // Used to capture error codes
#ifdef _WIN32
    #include <winsock2.h>     // Windows-specific networking functions
    #include <ws2tcpip.h>     // Windows-specific TCP/IP extensions
    #pragma comment(lib, "ws2_32.lib")  // Link the Winsock library for Windows
#else
    #include <arpa/inet.h>    // Unix/Linux-specific networking functions
#endif

// test_udp.c

#define PORT 8080  // Define the server port

// Function to handle client requests and send appropriate responses
void handleRequest(char *request, struct sockaddr_in cliaddr, int sockfd, socklen_t len) {
    char response[1024];  // Buffer to store the response

    // Parse and handle the client's request
    if (strncmp(request, "test_connection", 15) == 0) {
        // Handle a test connection request
        printf("Received test connection request from client\n");
        strcpy(response, "Connection OK");
    } else if (strncmp(request, "query_flight_id", 15) == 0) {
        // Handle a request for querying flight IDs
        printf("Received query_flight_id request\n");
        // Example: Return a dummy list of flight IDs
        strcpy(response, "Flight ID list: 123, 456, 789");
    } else if (strncmp(request, "query_flight_info", 17) == 0) {
        // Handle a request for querying flight information
        printf("Received query_flight_info request\n");
        // Example: Return dummy flight information
        strcpy(response, "Flight info: Flight 123, Departure: 10:00, Seats: 100");
    } else if (strncmp(request, "make_seat_reservation", 21) == 0) {
        // Handle a seat reservation request
        printf("Received make_seat_reservation request\n");
        // Example: Return confirmation of seat reservation
        strcpy(response, "Seat reservation confirmed");
    } else if (strncmp(request, "query_baggage_availability", 26) == 0) {
        // Handle a request to query baggage availability
        printf("Received query_baggage_availability request\n");
        // Example: Return dummy baggage availability information
        strcpy(response, "Baggage availability: 50");
    } else if (strncmp(request, "add_baggage", 11) == 0) {
        // Handle a request to add baggage
        printf("Received add_baggage request\n");
        // Example: Return confirmation of baggage addition
        strcpy(response, "Baggage added successfully");
    } else {
        // Handle unknown commands
        printf("Unknown command received: %s\n", request);
        strcpy(response, "Unknown command");
    }

    // Send the response back to the client
    if (sendto(sockfd, response, strlen(response), 0, (const struct sockaddr *)&cliaddr, len) < 0) {
        // If sending the response fails, output the error message
        perror("sendto failed");
    } else {
        // If successful, print the response sent to the client
        printf("Response sent to client: %s\n", response);
    }
}

int main() {
    int sockfd;  // Socket file descriptor
    char buffer[1024];  // Buffer to store incoming messages
    struct sockaddr_in servaddr, cliaddr;  // Structs to store server and client address info

    // Create a UDP socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        // If socket creation fails, print an error message and exit
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Clear memory for the server and client address structures
    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));

    // Set server address parameters
    servaddr.sin_family = AF_INET;  // IPv4
    servaddr.sin_addr.s_addr = INADDR_ANY;  // Bind to any available network interface
    servaddr.sin_port = htons(PORT);  // Bind to the specified port

    // Bind the socket to the server address and port
    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        // If binding fails, print an error message and exit
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    printf("Server is listening on port %d...\n", PORT);

    socklen_t len = sizeof(cliaddr);  // Get the length of the client address structure

    // Loop to continuously receive and handle client messages
    while (1) {
        // Receive a message from the client
        int n = recvfrom(sockfd, buffer, 1024, MSG_WAITALL, (struct sockaddr *)&cliaddr, &len);
        if (n < 0) {
            // If receiving the message fails, output an error message with the error code
            printf("recvfrom failed: %s (errno: %d)\n", strerror(errno), errno);
            continue;  // Continue to the next iteration to keep the server running
        }

        buffer[n] = '\0';  // Null-terminate the received message so it can be treated as a string
        printf("Received request: %s\n", buffer);

        // Handle the request by calling handleRequest()
        handleRequest(buffer, cliaddr, sockfd, len);
    }

    // Close the socket when done (in practice, this code will not be reached in an infinite loop)
    close(sockfd);
    return 0;
}
