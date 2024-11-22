#include <stdint.h>  // Provides fixed-width integer types like uint8_t and uint32_t
#include "server.h"   // Includes necessary server definitions and Flight structure
#include <stdio.h>    // Standard input/output functions
#include <string.h>   // String manipulation functions
#include <unistd.h>   // For sleep() function (POSIX)
#include <stdlib.h>   // For memory allocation and process control functions
#include <mysql/mysql.h>  // MySQL library for database interaction

#ifdef __linux__
// Includes necessary headers for socket programming on Linux
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#elif _WIN32
// Includes necessary headers for socket programming on Windows
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")  // Links Winsock library
#endif

#define BUFFER_SIZE 1024  // Defines buffer size for communication
#define MAX_LUGGAGE 400   // Maximum luggage capacity

// Month names used for formatting departure time
const char *months[] = {
    "January", "February", "March", "April", "May", "June",
    "July", "August", "September", "October", "November", "December"
};

// Function to handle flight queries based on source and destination (already modified)
void handle_query_flight(int sockfd, struct sockaddr_in *client_addr, char *request, MYSQL *conn) {
    char source[50], destination[50];  // Buffers to store source and destination strings
    int found = 0;  // Tracks if a matching flight is found

    // Extract source and destination from the client's request
    sscanf(request, "query_flight_id %s %s", source, destination);
    printf("Received query: source=%s, destination=%s\n", source, destination);

    // Build an SQL query to find matching flights
    char query[256];
    snprintf(query, sizeof(query),
             "SELECT flight_id FROM flights WHERE source_place='%s' AND destination_place='%s'",
             source, destination);

    // Execute the SQL query
    if (mysql_query(conn, query)) {
        // If the query fails, send an error message back to the client
        fprintf(stderr, "SELECT error: %s\n", mysql_error(conn));
        char response[BUFFER_SIZE];
        snprintf(response, sizeof(response), "Database query failed.\n");
        sendto(sockfd, response, strlen(response), 0, (struct sockaddr *)client_addr, sizeof(*client_addr));
        return;
    }

    MYSQL_RES *res = mysql_store_result(conn);  // Store the query result
    if (res == NULL) {
        // If storing the result fails, send an error message to the client
        fprintf(stderr, "mysql_store_result() failed: %s\n", mysql_error(conn));
        char response[BUFFER_SIZE];
        snprintf(response, sizeof(response), "Database error occurred.\n");
        sendto(sockfd, response, strlen(response), 0, (struct sockaddr *)client_addr, sizeof(*client_addr));
        return;
    }

    // Dynamically allocate memory for the response
    char *response = (char *)malloc(BUFFER_SIZE * sizeof(char));
    if (response == NULL) {
        // Handle memory allocation failure
        perror("Memory allocation failed");
        mysql_free_result(res);  // Free the query result
        return;
    }
    int response_size = BUFFER_SIZE;
    memset(response, 0, response_size);  // Initialize the response buffer to zero

    MYSQL_ROW row;  // Variable to hold each row of the result set
    while ((row = mysql_fetch_row(res))) {  // Fetch each row
        char flight_info[100];  // Buffer to store formatted flight information
        snprintf(flight_info, sizeof(flight_info), "Flight ID: %s\n", row[0]);

        // If the response buffer isn't large enough, dynamically expand it
        if (strlen(response) + strlen(flight_info) >= response_size) {
            response_size *= 2;  // Double the buffer size
            response = (char *)realloc(response, response_size * sizeof(char));
            if (response == NULL) {
                // Handle memory reallocation failure
                perror("Memory reallocation failed");
                mysql_free_result(res);  // Free the query result
                return;
            }
        }

        strcat(response, flight_info);  // Append flight information to the response
        found++;  // Increment the found counter
    }

    // If no matching flights were found, send an error message to the client
    if (!found) {
        strcpy(response, "No flights found.\n");
    }

    // Send the response to the client
    ssize_t sent_len = sendto(sockfd, response, strlen(response), 0, (struct sockaddr *)client_addr, sizeof(*client_addr));
    if (sent_len < 0) {
        // Handle potential errors in sending the response
        perror("Failed to send response");
    } else {
        printf("Response sent to client.\n");
        // Print log message
    }

    // Free dynamically allocated memory and the query result
    free(response);
    mysql_free_result(res);
}

// Function to handle detailed flight queries based on flight_id (already modified)
void handle_query_details(int sockfd, struct sockaddr_in *client_addr, char *request, MYSQL *conn) {
    int flight_id;
    int found = 0;
    char query[256];  // SQL query buffer
    char response[BUFFER_SIZE];  // Response buffer
    memset(response, 0, BUFFER_SIZE);  // Clear the response buffer

    // Extract the flight ID from the request
    sscanf(request, "query_flight_info %d", &flight_id);
    printf("Received query: flight_id=%d\n", flight_id);

    // Build the SQL query to retrieve flight details
    snprintf(query, sizeof(query), "SELECT flight_id, source_place, destination_place, departure_year, "
                                   "departure_month, departure_day, departure_hour, departure_minute, "
                                   "airfare, seat_availability, baggage_availability FROM flights WHERE flight_id = %d", flight_id);

    // Execute the SQL query
    if (mysql_query(conn, query)) {
        // Handle query failure and send error response
        fprintf(stderr, "SELECT error: %s\n", mysql_error(conn));
        snprintf(response, sizeof(response), "Database query failed.\n");
        sendto(sockfd, response, strlen(response), 0, (struct sockaddr *)client_addr, sizeof(*client_addr));
        return;
    }

    MYSQL_RES *res = mysql_store_result(conn);  // Store the query result
    if (res == NULL) {
        // Handle result storage failure and send error response
        fprintf(stderr, "mysql_store_result() failed: %s\n", mysql_error(conn));
        snprintf(response, sizeof(response), "Database error occurred.\n");
        sendto(sockfd, response, strlen(response), 0, (struct sockaddr *)client_addr, sizeof(*client_addr));
        return;
    }

    MYSQL_ROW row;  // Variable to hold each row of the result set

    // Loop through the query results
    while ((row = mysql_fetch_row(res))) {
        char departure_time[100];  // Buffer to hold the formatted departure time

        // Format the departure time as: Month day, year hour:minute
        snprintf(departure_time, sizeof(departure_time), "%s %02d, %d %02d:%02d",
                 months[atoi(row[4]) - 1],  // Month
                 atoi(row[5]),               // Day
                 atoi(row[3]),               // Year
                 atoi(row[6]),               // Hour
                 atoi(row[7]));              // Minute

        // Format and append the flight details to the response
        snprintf(response + strlen(response), sizeof(response) - strlen(response),
                 "Flight ID: %s\n"
                 "Source: %s\n"
                 "Destination: %s\n"
                 "Departure Time: %s\n"
                 "Airfare: %s\n"
                 "Seats Available: %s\n"
                 "Baggage Availability: %s kg\n\n",
                 row[0],  // flight_id
                 row[1],  // source_place
                 row[2],  // destination_place
                 departure_time,  // Formatted departure time
                 row[8],  // airfare
                 row[9], // seat_availability
                 row[10]  // baggage_availability
        );

        found = 1;  // Mark the flight as found
    }

    // If no flight is found, send an error response
    if (!found) {
        snprintf(response, sizeof(response), "Flight not found.\n");
    }
    store_in_history(client_addr, request, response);  // Store the response in the request history

    // Send the response to the client
    sendto(sockfd, response, strlen(response), 0, (struct sockaddr *)client_addr, sizeof(*client_addr));

    // Free the query result
    mysql_free_result(res);

    // Log the response
    printf("Response sent to client.\n");
}

// Function to handle seat reservation requests
void handle_reservation(int sockfd, struct sockaddr_in *client_addr, char *request, MYSQL *conn) {
    int flight_id, seats;  // Variables to hold flight ID and number of seats to reserve
    char query[256];  // SQL query buffer
    char response[BUFFER_SIZE];  // Response buffer
    memset(response, 0, BUFFER_SIZE);  // Clear the response buffer

    // Extract flight ID and seat count from the client's request
    sscanf(request, "make_seat_reservation %d %d", &flight_id, &seats);
    printf("Received reservation request: Flight ID=%d, Seats=%d\n", flight_id, seats);

    // Build an SQL query to check available seats
    snprintf(query, sizeof(query), "SELECT seat_availability FROM flights WHERE flight_id=%d", flight_id);

    // Execute the SQL query
    if (mysql_query(conn, query)) {
        // Handle query failure and send error response
        fprintf(stderr, "SELECT error: %s\n", mysql_error(conn));
        snprintf(response, sizeof(response), "Database query failed.\n");
        sendto(sockfd, response, strlen(response), 0, (struct sockaddr *)client_addr, sizeof(*client_addr));
        return;
    }

    MYSQL_RES *res = mysql_store_result(conn);  // Store the query result
    if (res == NULL) {
        // Handle result storage failure and send error response
        fprintf(stderr, "mysql_store_result() failed: %s\n", mysql_error(conn));
        snprintf(response, sizeof(response), "Database error occurred.\n");
        sendto(sockfd, response, strlen(response), 0, (struct sockaddr *)client_addr, sizeof(*client_addr));
        return;
    }

    MYSQL_ROW row = mysql_fetch_row(res);  // Fetch the result row

    if (row) {
        int seat_availability = atoi(row[0]);  // Get available seat count
        if (seat_availability == 0) {
            // If no seats are available, send a failure response
            strcpy(response, "Reservation failed: No seats available.\n");
        } else if (seat_availability < seats) {
            // If not enough seats are available, send a failure response
            strcpy(response, "Reservation failed: Not enough seats available. Reduce your reservation.\n");
        } else {
            // If seats are available, update the seat availability
            int new_seat_availability = seat_availability - seats;
            snprintf(query, sizeof(query),
                     "UPDATE flights SET seat_availability=%d WHERE flight_id=%d",
                     new_seat_availability, flight_id);

            // Execute the update query
            if (mysql_query(conn, query)) {
                // Handle query failure and send an error response
                fprintf(stderr, "UPDATE error: %s\n", mysql_error(conn));
                snprintf(response, sizeof(response), "Database update failed.\n");
            } else {
                // Send a confirmation response with the remaining seat count
                snprintf(response, sizeof(response),
                         "Reservation confirmed for Flight ID: %d\nSeats remaining: %d\n",
                         flight_id, new_seat_availability);
            }
        }
    } else {
        // If no matching flight is found, send an error response
        strcpy(response, "Flight not found.\n");
    }

    // Free the query result
    mysql_free_result(res);
    store_in_history(client_addr, request, response);  // Store the response in the request history

    // Send the response to the client
    sendto(sockfd, response, strlen(response), 0, (struct sockaddr *)client_addr, sizeof(*client_addr));

    // Log the response
    printf("Response sent to client.\n");
}

// Function to handle baggage addition requests
void handle_add_baggage(int sockfd, struct sockaddr_in *client_addr, char *request, MYSQL *conn) {
    int flight_id, baggages;  // Variables to hold flight ID and baggage count
    char query[256];  // SQL query buffer
    char response[BUFFER_SIZE];  // Response buffer
    memset(response, 0, BUFFER_SIZE);  // Clear the response buffer

    // Extract flight ID and baggage count from the request
    sscanf(request, "add_baggage %d %d", &flight_id, &baggages);
    printf("Received baggage reservation request: Flight ID=%d, Baggages=%d\n", flight_id, baggages);

    // Build an SQL query to check available baggage space
    snprintf(query, sizeof(query), "SELECT baggage_availability FROM flights WHERE flight_id=%d", flight_id);

    // Execute the SQL query
    if (mysql_query(conn, query)) {
        // Handle query failure and send error response
        fprintf(stderr, "SELECT error: %s\n", mysql_error(conn));
        snprintf(response, sizeof(response), "Database query failed.\n");
        sendto(sockfd, response, strlen(response), 0, (struct sockaddr *)client_addr, sizeof(*client_addr));
        return;
    }

    MYSQL_RES *res = mysql_store_result(conn);  // Store the query result
    if (res == NULL) {
        // Handle result storage failure and send error response
        fprintf(stderr, "mysql_store_result() failed: %s\n", mysql_error(conn));
        snprintf(response, sizeof(response), "Database error occurred.\n");
        sendto(sockfd, response, strlen(response), 0, (struct sockaddr *)client_addr, sizeof(*client_addr));
        return;
    }

    MYSQL_ROW row = mysql_fetch_row(res);  // Fetch the result row

    if (row) {
        int baggage_availability = atoi(row[0]);  // Get available baggage space
        if (baggage_availability == 0) {
            // If no baggage space is available, send a failure response
            strcpy(response, "Baggage reservation failed: No baggage space available.\n");
        } else if (baggage_availability < baggages) {
            // If not enough baggage space is available, send a failure response
            strcpy(response, "Baggage reservation failed: Not enough space for baggage. Reduce your request.\n");
        } else {
            // If space is available, update the baggage availability
            int new_baggage_availability = baggage_availability - baggages;
            snprintf(query, sizeof(query),
                     "UPDATE flights SET baggage_availability=%d WHERE flight_id=%d",
                     new_baggage_availability, flight_id);

            // Execute the update query
            if (mysql_query(conn, query)) {
                // Handle query failure and send an error response
                fprintf(stderr, "UPDATE error: %s\n", mysql_error(conn));
                snprintf(response, sizeof(response), "Database update failed.\n");
            } else {
                // Send a confirmation response with the remaining baggage space
                snprintf(response, sizeof(response),
                         "Baggage reservation confirmed for Flight ID: %d\nBaggage space remaining: %d\n",
                         flight_id, new_baggage_availability);
            }
        }
    } else {
        // If no matching flight is found, send an error response
        strcpy(response, "Flight not found.\n");
    }

    // Free the query result
    mysql_free_result(res);
    store_in_history(client_addr, request, response);  // Store the response in the request history

    // Send the response to the client
    sendto(sockfd, response, strlen(response), 0, (struct sockaddr *)client_addr, sizeof(*client_addr));

    // Log the response
    printf("Response sent to client.\n");
}

// Function to handle baggage availability queries
void handle_query_baggage_availability(int sockfd, struct sockaddr_in *client_addr, char *request, MYSQL *conn) {
    int flight_id;  // Variable to hold flight ID
    char query[256];  // SQL query buffer
    char response[BUFFER_SIZE];  // Response buffer
    memset(response, 0, BUFFER_SIZE);  // Clear the response buffer

    // Extract flight ID from the request
    sscanf(request, "query_baggage_availability %d", &flight_id);
    printf("Received query for baggage availability: Flight ID=%d\n", flight_id);

    // Build an SQL query to retrieve baggage availability
    snprintf(query, sizeof(query), "SELECT baggage_availability FROM flights WHERE flight_id = %d", flight_id);

    // Execute the SQL query
    if (mysql_query(conn, query)) {
        // Handle query failure and send error response
        fprintf(stderr, "SELECT error: %s\n", mysql_error(conn));
        snprintf(response, sizeof(response), "Database query failed.\n");
        sendto(sockfd, response, strlen(response), 0, (struct sockaddr *)client_addr, sizeof(*client_addr));
        return;
    }

    MYSQL_RES *res = mysql_store_result(conn);  // Store the query result
    if (res == NULL) {
        // Handle result storage failure and send error response
        fprintf(stderr, "mysql_store_result() failed: %s\n", mysql_error(conn));
        snprintf(response, sizeof(response), "Database error occurred.\n");
        sendto(sockfd, response, strlen(response), 0, (struct sockaddr *)client_addr, sizeof(*client_addr));
        return;
    }

    MYSQL_ROW row = mysql_fetch_row(res);  // Fetch the result row

    if (row) {
        // Retrieve baggage availability and format the response
        snprintf(response, sizeof(response), "Flight ID: %d\nBaggage space available: %s\n", flight_id, row[0]);
    } else {
        // If no matching flight is found, send an error response
        strcpy(response, "Flight not found.\n");
    }

    // Free the query result
    mysql_free_result(res);
    store_in_history(client_addr, request, response);  // Store the response in the request history

    // Send the response to the client
    sendto(sockfd, response, strlen(response), 0, (struct sockaddr *)client_addr, sizeof(*client_addr));

    // Log the response
    printf("Response sent to client.\n");
}

