#include <stdint.h>  // Provides fixed-width integer types like uint8_t and uint32_t
#include "server.h"  // This must include the Flight structure definition
#include <mysql/mysql.h>  // MySQL library for database interaction
#include <stdio.h>  // Standard input/output functions
#include <stdlib.h>  // Standard library functions like memory allocation
#include <string.h>  // String manipulation functions

// Database connection information
#define HOST "localhost"  // MySQL server host
#define USER "root"       // MySQL user name
#define PASS "root"       // MySQL password
#define DB "flight_system"  // Database name

// Function to connect to the MySQL database
MYSQL* connect_db() {
    MYSQL *conn = mysql_init(NULL);  // Initialize a MySQL connection handler
    if (conn == NULL) {  // If initialization fails, print an error message and exit
        printf("mysql_init() failed\n");
        exit(EXIT_FAILURE);
    }

    // Establish a connection to the database
    if (mysql_real_connect(conn, HOST, USER, PASS, DB, 0, NULL, 0) == NULL) {
        // If the connection fails, print an error message, close the connection, and exit
        printf("mysql_real_connect() failed: %s\n", mysql_error(conn));
        mysql_close(conn);  // Close the MySQL connection
        exit(EXIT_FAILURE);
    }

    return conn;  // Return the connected MySQL handler
}

// Function to query flight data from the database
void query_flights(MYSQL *conn) {
    const char *query = "SELECT flight_id, source_place, destination_place, "
                        "departure_year, departure_month, departure_day, "
                        "departure_hour, departure_minute, airfare, "
                        "seat_availability, baggage_availability FROM flights";

    // Execute the query on the MySQL connection
    if (mysql_query(conn, query)) {
        printf("QUERY failed: %s\n", mysql_error(conn));  // Print an error if the query fails
        return;
    }

    MYSQL_RES *result = mysql_store_result(conn);  // Store the result of the query
    if (result == NULL) {
        printf("mysql_store_result() failed: %s\n", mysql_error(conn));  // Print an error if the result is null
        return;
    }

    MYSQL_ROW row;  // Row structure to hold each row of the result set
    while ((row = mysql_fetch_row(result))) {  // Fetch each row from the result
        Flight flight;  // Declare a flight structure to store flight data

        flight.flight_id = atoi(row[0]);  // Convert flight_id from string to integer
        flight.source_place = strdup(row[1]);  // Duplicate the source_place string
        flight.destination_place = strdup(row[2]);  // Duplicate the destination_place string

        // Populate the DepartureTime structure with year, month, day, hour, and minute
        flight.departure_time.year = atoi(row[3]);
        flight.departure_time.month = atoi(row[4]);
        flight.departure_time.day = atoi(row[5]);
        flight.departure_time.hour = atoi(row[6]);
        flight.departure_time.minute = atoi(row[7]);

        flight.airfare = atof(row[8]);  // Convert airfare from string to float
        flight.seat_availability = atoi(row[9]);  // Convert seat availability from string to integer
        flight.baggage_availability = atoi(row[10]);  // Convert baggage availability from string to integer

    }

    mysql_free_result(result);  // Free the result set after processing all rows
}

// Function to update seat availability for a specific flight
void update_seats(MYSQL *conn, int flight_id, int seats_reserved) {
    char query[256];  // Buffer to hold the SQL query

    // First, query the current seat availability
    sprintf(query, "SELECT seat_availability FROM flights WHERE flight_id = %d", flight_id);
    if (mysql_query(conn, query)) {  // Execute the query
        printf("SELECT QUERY failed: %s\n", mysql_error(conn));  // Print error message if query fails
        return;
    }

    MYSQL_RES *result = mysql_store_result(conn);  // Store the result of the query
    if (result == NULL) {
        printf("mysql_store_result() failed: %s\n", mysql_error(conn));  // Print error if result is null
        return;
    }

    MYSQL_ROW row = mysql_fetch_row(result);  // Fetch the first row
    if (row) {
        int available_seats = atoi(row[0]);  // Get the current seat availability
        if (available_seats < seats_reserved) {
            printf("Not enough seats available.\n");  // Print message if there are insufficient seats
        } else {
            // If enough seats are available, update the seat count
            sprintf(query, "UPDATE flights SET seat_availability = seat_availability - %d WHERE flight_id = %d", seats_reserved, flight_id);
            if (mysql_query(conn, query)) {  // Execute the update query
                printf("UPDATE QUERY failed: %s\n", mysql_error(conn));  // Print error if the update fails
            } else {
                printf("Seats updated successfully.\n");  // Print success message
            }
        }
    } else {
        printf("Flight not found.\n");  // Print message if flight ID is not found
    }

    mysql_free_result(result);  // Free the result set
}

// Function to update baggage availability for a specific flight
void update_baggage(MYSQL *conn, int flight_id, int baggage_added) {
    char query[256];  // Buffer to hold the SQL query

    // Query the current baggage availability
    sprintf(query, "SELECT baggage_availability FROM flights WHERE flight_id = %d", flight_id);
    if (mysql_query(conn, query)) {  // Execute the query
        printf("SELECT QUERY failed: %s\n", mysql_error(conn));  // Print error if query fails
        return;
    }

    MYSQL_RES *result = mysql_store_result(conn);  // Store the result of the query
    if (result == NULL) {
        printf("mysql_store_result() failed: %s\n", mysql_error(conn));  // Print error if result is null
        return;
    }

    MYSQL_ROW row = mysql_fetch_row(result);  // Fetch the first row
    if (row) {
        int available_baggage = atoi(row[0]);  // Get the current baggage availability
        if (available_baggage < baggage_added) {
            printf("Not enough baggage space available.\n");  // Print message if insufficient baggage space
        } else {
            // Update the baggage count if enough space is available
            sprintf(query, "UPDATE flights SET baggage_availability = baggage_availability - %d WHERE flight_id = %d", baggage_added, flight_id);
            if (mysql_query(conn, query)) {  // Execute the update query
                printf("UPDATE QUERY failed: %s\n", mysql_error(conn));  // Print error if update fails
            } else {
                printf("Baggage updated successfully.\n");  // Print success message
            }
        }
    } else {
        printf("Flight not found.\n");  // Print message if flight ID is not found
    }

    mysql_free_result(result);  // Free the result set
}

// Function to close the MySQL database connection
void close_db(MYSQL *conn) {
    mysql_close(conn);  // Close the MySQL connection
}

