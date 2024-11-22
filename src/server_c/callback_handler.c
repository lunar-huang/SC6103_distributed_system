#include <stdint.h> // Add this header to define uint8_t and uint32_t
#include <stdio.h>  // Standard input-output for printf and snprintf
#include <string.h> // For string manipulation functions like strncpy
#include <unistd.h> // For sleep function

#ifdef _WIN32
#include <winsock2.h>  // Windows-specific socket library
#include <ws2tcpip.h>  // Windows-specific TCP/IP functions
#include <windows.h>   // General Windows functions
#pragma comment(lib, "ws2_32.lib")  // Link with ws2_32.lib for Windows socket support
#else
#include <arpa/inet.h> // For inet_ntop and other network functions on Unix
#include <netinet/in.h> // For sockaddr_in struct used in networking
#include <sys/socket.h> // For socket functions
#include <sys/types.h>  // For system types, used in networking
#endif

#include <pthread.h>  // For pthread functions to enable multithreading
#include "server.h"   // Custom header file that contains project-specific declarations

// callback_handler.c

#define BUFFER_SIZE 1024  // Define the size of the buffer for data communication

// Structure for monitoring registered clients
typedef struct
{
    struct sockaddr_in client_addr;  // Store client's address (IP, port)
    int flight_id;                   // Flight ID the client is monitoring
    int seat_availability;           // Current seat availability on the flight
} ClientMonitor;

ClientMonitor client_monitors[100];  // Array to store up to 100 monitored clients
int client_monitor_count = 0;        // Number of clients currently being monitored

extern pthread_mutex_t flight_mutex; // Mutex for ensuring thread safety while accessing shared data

/**
 * @brief Register a client to monitor a specific flight's seat availability.
 * @param sockfd The socket file descriptor for communication.
 * @param client_addr The client's network address.
 * @param flight_id The ID of the flight the client wants to monitor.
 */
void register_flight_monitor(int sockfd, struct sockaddr_in *client_addr, int flight_id)
{
    // Lock the mutex to ensure thread-safe access to shared data
    pthread_mutex_lock(&flight_mutex);

    // Register the client by storing its address and the flight it wants to monitor
    client_monitors[client_monitor_count].client_addr = *client_addr;
    client_monitors[client_monitor_count].flight_id = flight_id;
    client_monitor_count++;  // Increment the count of monitored clients

    // Unlock the mutex after the shared data has been updated
    pthread_mutex_unlock(&flight_mutex);

    // Send a response to the client confirming successful registration
    char response[BUFFER_SIZE];
    sprintf(response, "Registered for flight %d seat availability updates\n", flight_id);
    sendto(sockfd, response, strlen(response), 0, (struct sockaddr *)client_addr, sizeof(*client_addr));
}

/**
 * @brief Thread function that monitors flights and notifies clients of seat availability changes.
 * @param arg A pointer to the client data structure.
 * @return NULL
 */
void* monitor_flights(void* arg)
{
    struct client_data *data = (struct client_data *)arg;  // Cast the argument to the correct type
    MYSQL *conn = connect_db();  // Establish a connection to the MySQL database

    while (1)  // Infinite loop to continuously monitor flights
    {
        // Lock the mutex to ensure only one thread accesses shared data at a time
        pthread_mutex_lock(&flight_mutex);

        // Loop through all the monitored clients
        for (int i = 0; i < client_monitor_count; i++)
        {
            int flight_id = client_monitors[i].flight_id;  // Get the flight ID being monitored by the client

            // Formulate an SQL query to get the current seat availability for the flight
            char query[256];
            snprintf(query, sizeof(query), "SELECT seat_availability FROM flights WHERE flight_id = %d", flight_id);

            // Execute the query and check for errors
            if (mysql_query(conn, query))
            {
                fprintf(stderr, "SELECT error: %s\n", mysql_error(conn));
                continue;  // Skip to the next client if there is an error
            }

            // Store the result of the query
            MYSQL_RES *res = mysql_store_result(conn);
            if (res == NULL)
            {
                fprintf(stderr, "mysql_store_result() failed: %s\n", mysql_error(conn));
                continue;
            }

            // Fetch the row containing the seat availability
            MYSQL_ROW row = mysql_fetch_row(res);
            if (row)  // If the row exists
            {
                int current_seat_availability = atoi(row[0]);  // Convert seat availability to integer

                // Compare the current seat availability with the last known value for the client
                if (client_monitors[i].seat_availability != current_seat_availability)
                {
                    // Update the seat availability for the client
                    client_monitors[i].seat_availability = current_seat_availability;
                    printf("Seats availability changed for flight %d, notifying clients\n", flight_id);

                    // Notify all clients monitoring this flight of the seat availability update
                    for (int j = 0; j < client_monitor_count; j++)
                    {
                        if (client_monitors[j].flight_id == flight_id)
                        {
                            // Create a response message to send to the client
                            char response[BUFFER_SIZE];
                            snprintf(response, sizeof(response),
                                     "Flight %d seat availability updated to %d\n",
                                     flight_id, current_seat_availability);

                            // Send the message to the client
                            ssize_t sent_len = sendto(data->sockfd, response, strlen(response), 0,
                                                      (struct sockaddr *)&client_monitors[j].client_addr,
                                                      sizeof(client_monitors[j].client_addr));

                            // Check if the message was sent successfully
                            if (sent_len == -1)
                            {
                                perror("Failed to send data with sendto");  // Error handling for send failure
                            }
                            else
                            {
                                // Print the number of bytes sent successfully
                                printf("Successfully sent %ld bytes to the client\n", sent_len);
                            }
                        }
                    }
                }
            }
            mysql_free_result(res);  // Free the memory used by the result set
        }

        // Unlock the mutex after the shared data access is complete
        pthread_mutex_unlock(&flight_mutex);

        sleep(5);  // Pause for 5 seconds before the next iteration of monitoring
    }

    close_db(conn);  // Close the connection to the database
    return NULL;  // Return NULL as this function is used in a thread
}


