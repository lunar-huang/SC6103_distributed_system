#include <stdint.h>  // To define uint8_t and uint32_t for platform-independent integer types
#include "server.h"  // Custom header file that defines structures and functions related to the server
#include <string.h>  // Provides string manipulation functions
#include <stdio.h>   // Provides input/output functions like printf and perror
#include <stdlib.h>  // Provides memory allocation and control functions like malloc and free

// Function to initialize the flight data with an initial capacity for storage
void initialize_flights(int initial_capacity) {
    max_flights = initial_capacity;  // Set the maximum number of flights we can handle initially
    flights = (Flight *)malloc(max_flights * sizeof(Flight));  // Allocate memory for storing flight data
    if (flights == NULL) {  // Check if memory allocation failed
        perror("Memory allocation failed for flights");  // Print error message to stderr
        exit(EXIT_FAILURE);  // Exit the program if memory allocation fails
    }

    // Initialize example flight data manually for demonstration purposes
    flights[0].flight_id = 1;  // Assign flight ID
    flights[0].source_place = malloc(strlen("Singapore") + 1);  // Allocate memory for the source place string
    flights[0].destination_place = malloc(strlen("Tokyo") + 1);  // Allocate memory for the destination place string
    if (flights[0].source_place && flights[0].destination_place) {  // Check if both memory allocations succeeded
        strcpy(flights[0].source_place, "Singapore");  // Copy the source place string into the allocated memory
        strcpy(flights[0].destination_place, "Tokyo");  // Copy the destination place string into the allocated memory
        flights[0].departure_time = (DepartureTime){2024, 10, 12, 8, 0};  // Set the flight's departure time
        flights[0].airfare = 500.0;  // Set the airfare for the flight
        flights[0].seat_availability = 50;  // Set the number of available seats
        flights[0].baggage_availability = 100;  // Set the number of available baggage slots
    } else {
        perror("Memory allocation failed for flight strings");  // Error handling for string allocation failure
        exit(EXIT_FAILURE);  // Exit if allocation fails
    }

    // Initialize a second example flight
    flights[1].flight_id = 2;  // Assign flight ID
    flights[1].source_place = malloc(strlen("Singapore") + 1);  // Allocate memory for source place
    flights[1].destination_place = malloc(strlen("New York") + 1);  // Allocate memory for destination place
    if (flights[1].source_place && flights[1].destination_place) {  // Check if allocation succeeded
        strcpy(flights[1].source_place, "Singapore");  // Copy source place string
        strcpy(flights[1].destination_place, "New York");  // Copy destination place string
        flights[1].departure_time = (DepartureTime){2024, 10, 13, 23, 0};  // Set departure time
        flights[1].airfare = 1200.0;  // Set airfare
        flights[1].seat_availability = 30;  // Set seat availability
        flights[1].baggage_availability = 50;  // Set baggage availability
    } else {
        perror("Memory allocation failed for flight strings");  // Handle memory allocation failure
        exit(EXIT_FAILURE);  // Exit if allocation fails
    }

    flight_count = 2;  // Set initial flight count to 2 since two flights are initialized
}

// Function to find a flight by its ID
Flight* find_flight_by_id(int flight_id) {
    for (int i = 0; i < flight_count; i++) {  // Loop through all the flights
        if (flights[i].flight_id == flight_id) {  // Check if the current flight matches the requested flight ID
            return &flights[i];  // Return the pointer to the matching flight
        }
    }
    return NULL;  // Return NULL if no flight is found with the given ID
}

// Function to update the seat availability for a specific flight
int update_flight_seats(int flight_id, int seats) {
    Flight *flight = find_flight_by_id(flight_id);  // Find the flight by ID
    if (flight != NULL) {  // If the flight is found
        if (flight->seat_availability >= seats) {  // Check if there are enough seats available
            flight->seat_availability -= seats;  // Reduce the seat availability by the requested number of seats
            return 1;  // Return 1 to indicate a successful update
        } else {
            return -1;  // Return -1 if there aren't enough seats available
        }
    }
    return 0;  // Return 0 if the flight was not found
}

// Function to add a new flight to the system
int add_flight(int flight_id, const char *source, const char *destination, 
               DepartureTime departure_time, float airfare, 
               int seat_availability, int baggage_availability) {
    if (flight_count >= max_flights) {  // Check if the current flight count exceeds the allocated space
        max_flights *= 2;  // Double the flight capacity
        flights = (Flight *)realloc(flights, max_flights * sizeof(Flight));  // Reallocate memory for more flights
        if (flights == NULL) {  // Check if reallocation failed
            perror("Memory reallocation failed");  // Print error message
            return -1;  // Return -1 to indicate failure
        }
    }

    flights[flight_count].flight_id = flight_id;  // Set the flight ID

    // Allocate memory for and copy the source and destination strings
    flights[flight_count].source_place = malloc(strlen(source) + 1);
    flights[flight_count].destination_place = malloc(strlen(destination) + 1);
    if (flights[flight_count].source_place == NULL || flights[flight_count].destination_place == NULL) {
        perror("Memory allocation failed for flight strings");  // Handle memory allocation failure
        return -1;  // Return -1 if memory allocation fails
    }
    strcpy(flights[flight_count].source_place, source);  // Copy source place string
    strcpy(flights[flight_count].destination_place, destination);  // Copy destination place string

    // Set the rest of the flight details
    flights[flight_count].departure_time = departure_time;  // Set the departure time
    flights[flight_count].airfare = airfare;  // Set the airfare
    flights[flight_count].seat_availability = seat_availability;  // Set the seat availability
    flights[flight_count].baggage_availability = baggage_availability;  // Set the baggage availability
    flight_count++;  // Increment the flight count

    return 1;  // Return 1 to indicate successful flight addition
}

// Function to clean up allocated memory for flight data
void cleanup_flights() {
    if (flights != NULL) {  // Check if the flights array is not NULL
        for (int i = 0; i < flight_count; i++) {  // Loop through all the flights
            free(flights[i].source_place);  // Free the memory allocated for the source place
            free(flights[i].destination_place);  // Free the memory allocated for the destination place
        }
        free(flights);  // Free the memory allocated for the flights array
        flights = NULL;  // Set the flights pointer to NULL to avoid dangling pointers
    }
}

