// handleRequest.c
#include <stdio.h>
#include <string.h>
#include <mysql/mysql.h>  // MySQL library for database operations

// Include necessary headers based on the operating system
#ifdef __linux__
#include <arpa/inet.h>     // For socket functions in Linux
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#elif _WIN32
#include <winsock2.h>      // For socket functions in Windows
#include <ws2tcpip.h>
#include <windows.h>
#pragma comment(lib, "ws2_32.lib")  // Link Windows socket library
#endif

#include <pthread.h>  // For threading functionality
#include "server.h"   // Include the server header, which contains relevant function declarations

// Function to handle different client requests
void handleRequest(char *request, struct sockaddr_in cliaddr, int sockfd, socklen_t len, MYSQL *conn) {
    char response[1024];  // Buffer to store the response sent back to the client

    // Parse the client's request and handle different types of requests accordingly
    if (strncmp(request, "test_connection", 15) == 0) {
        // Handle a "test_connection" request to verify the server is reachable
        printf("Received test connection request from client\n");
        strcpy(response, "Connection OK");  // Simple response to confirm connection
    } 
    else if (strncmp(request, "query_flight_id", 15) == 0) {
        // Handle a request to query flight IDs based on source and destination
        printf("Received query_flight_id request\n");
        handle_query_flight(sockfd, &cliaddr, request, conn);  // Call function to handle flight ID query
    } 
    else if (strncmp(request, "query_flight_info", 17) == 0) {
        // Handle a request to get detailed flight information
        printf("Received query_flight_info request\n");
        handle_query_details(sockfd, &cliaddr, request, conn);  // Call function to handle detailed flight info query
    } 
    else if (strncmp(request, "make_seat_reservation", 21) == 0) {
        // Handle a request to make a seat reservation
        printf("Received make_seat_reservation request\n");
        handle_reservation(sockfd, &cliaddr, request, conn);  // Call function to handle seat reservation
    } 
    else if (strncmp(request, "query_baggage_availability", 26) == 0) {
        // Handle a request to check baggage availability
        printf("Received query_baggage_availability request\n");
        handle_query_baggage_availability(sockfd, &cliaddr, request, conn);  // Call function to handle baggage availability query
    } 
    else if (strncmp(request, "add_baggage", 11) == 0) {
        // Handle a request to add baggage to a flight
        printf("Received add_baggage request\n");
        handle_add_baggage(sockfd, &cliaddr, request, conn);  // Call function to handle baggage addition
    } 
    else if (strncmp(request, "follow_flight_id", 16) == 0) {
        // Handle a request to start monitoring a flight
        printf("Received follow_flight_id request\n");

        // Parse the flight ID from the request
        int flight_id;
        sscanf(request, "follow_flight_id %d", &flight_id);  // Extract flight ID from the request string
        printf("Received follow_flight_id request for flight_id: %d\n", flight_id);

        // Register the client for monitoring the specified flight
        register_flight_monitor(sockfd, &cliaddr, flight_id);

        // Create a new thread to monitor flight seat availability
        pthread_t monitor_thread;
        struct client_data *data = malloc(sizeof(struct client_data));  // Allocate memory for client data
        data->sockfd = sockfd;  // Store socket file descriptor
        data->client_addr = cliaddr;  // Store client address
        data->conn = conn;  // Store database connection

        // Create a thread to monitor the flight's seat availability
        if (pthread_create(&monitor_thread, NULL, monitor_flights, (void *)data) != 0) {
            perror("Failed to create monitor thread");  // Error handling if thread creation fails
        }
        pthread_detach(monitor_thread);  // Detach the thread so it can run independently

        // Send a confirmation response to the client
        strcpy(response, "Flight monitoring started.\n");
    } 
    else {
        // Handle an unknown or unsupported command
        printf("Unknown command received: %s\n", request);
        strcpy(response, "Unknown command");  // Respond with an error message
        sendto(sockfd, response, strlen(response), 0, (const struct sockaddr *)&cliaddr, len);  // Send response to client
    }

    // Log the response sent to the client
    // printf("Response sent to client: %s\n", response);
    printf("Response sent to client.\n");
}


