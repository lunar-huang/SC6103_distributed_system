#ifndef SERVER_H
#define SERVER_H

// Common platform-specific includes for networking
#if defined(__linux__) || defined(__APPLE__)
#include <netinet/in.h>    // For sockaddr_in, IP addresses, etc. (Linux/Unix)
#include <arpa/inet.h>     // For inet_addr, converting IP address formats (Linux/Unix)
#include <sys/socket.h>    // For socket operations (Linux/Unix)
#elif defined(_WIN32)
#include <winsock2.h>      // Windows socket functions (for networking)
#include <ws2tcpip.h>      // Windows TCP/IP extensions
#pragma comment(lib, "ws2_32.lib")  // Link Windows socket library
#endif

#include <pthread.h>       // For threading support
#include <stdint.h>        // For uint8_t and uint32_t types
#include <mysql/mysql.h>   // MySQL database interaction

#define BUFFER_SIZE 1024   // Define buffer size for communication

// Structure to store the departure time of a flight
typedef struct {
    int year;     // Year of departure
    int month;    // Month of departure (1-12)
    int day;      // Day of the month of departure (1-31)
    int hour;     // Hour of departure (0-23)
    int minute;   // Minute of departure (0-59)
} DepartureTime;

// Structure to store information about a flight
typedef struct {
    int flight_id;               // Unique identifier for the flight
    char *source_place;          // Departure location
    char *destination_place;     // Arrival location
    DepartureTime departure_time; // Struct to hold the flight's departure time
    float airfare;               // Price of the flight
    int seat_availability;       // Number of available seats
    int baggage_availability;    // Available baggage space
} Flight;

// Structure to store client-specific data for each connection
struct client_data {
    char buffer[BUFFER_SIZE];    // Data buffer for client communication
    struct sockaddr_in client_addr;  // Client address information
    int sockfd;                  // Socket file descriptor
    socklen_t addr_len;          // Length of client address structure
    MYSQL *conn;                 // MySQL database connection for this client
};

// Declare variables for flight information
extern Flight *flights;          // Pointer to an array of flights
extern int flight_count;         // Number of currently stored flights
extern int max_flights;          // Maximum capacity of the flight array

// Callback handling declarations
void handle_client_request(int sockfd, struct sockaddr_in *client_addr, char *buffer, MYSQL *conn);  // Handle client request
void register_flight_monitor(int sockfd, struct sockaddr_in *client_addr, int flight_id);  // Register client to monitor a flight
void* monitor_flights(void* arg);  // Monitor flight updates in a separate thread
Flight* unmarshal_flight(const uint8_t* buffer, uint32_t* flight_data_length);  // Unmarshal flight data from a byte array

// Data storage declarations
void initialize_flights();        // Initialize the flight array with sample data
Flight* find_flight_by_id(int flight_id);  // Find a flight by its ID
int update_flight_seats(int flight_id, int seats);  // Update the number of available seats for a flight
int add_flight(int flight_id, const char *source, const char *destination, DepartureTime departure_time, float airfare, int seat_availability, int baggage_availability);  // Add a new flight to the system

// Flight service function declarations (for handling specific flight-related requests)
void handle_query_flight(int sockfd, struct sockaddr_in *client_addr, char *request, MYSQL *conn);  // Handle query flight by source and destination
void handle_query_details(int sockfd, struct sockaddr_in *client_addr, char *request, MYSQL *conn);  // Handle detailed flight info query by flight ID
void handle_reservation(int sockfd, struct sockaddr_in *client_addr, char *request, MYSQL *conn);  // Handle seat reservation request
void handle_add_baggage(int sockfd, struct sockaddr_in *client_addr, char *request, MYSQL *conn);  // Handle baggage addition request
void handle_query_baggage_availability(int sockfd, struct sockaddr_in *client_addr, char *request, MYSQL *conn);  // Handle query for baggage availability

// Thread pool function declarations (if thread pooling is implemented in the system)
void thread_pool_init(int num_threads);  // Initialize a thread pool with a given number of threads
void thread_pool_add_task(void (*function)(void *), void *arg);  // Add a task to the thread pool
void thread_pool_destroy();  // Clean up and destroy the thread pool

// Server request handling declarations
void handleRequest(char *request, struct sockaddr_in cliaddr, int sockfd, socklen_t len, MYSQL *conn);  // Main handler for processing client requests
void store_in_history(struct sockaddr_in* client_addr, const char* request, const char* response);  // Store processed requests in history (for at-most-once processing)
int find_in_history(int sockfd, struct sockaddr_in *client_addr, const char *request, char *response);  // Check if a request has been processed before (for at-most-once processing)
void* handle_client(void* arg);  // Thread function to handle individual client requests

// Networking utility function declarations
void set_nonblocking(int sockfd);  // Set a socket to non-blocking mode

// Database connection handling declarations
MYSQL* connect_db();  // Connect to the MySQL database
void close_db(MYSQL *conn);  // Close the database connection
void query_flights(MYSQL *conn);  // Query flight data from the database
void update_seats(MYSQL *conn, int flight_id, int seats_reserved);  // Update the seat availability in the database
void update_baggage(MYSQL *conn, int flight_id, int baggage_added);  // Update baggage availability in the database

#endif // SERVER_H
