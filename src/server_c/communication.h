#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include <stdint.h>  // Standard library for fixed-width integer types
#include "server.h"  // Custom header file that defines server-related structures and functions

#ifdef _WIN32
    #include <winsock2.h>   // Windows-specific socket library
    #pragma comment(lib, "ws2_32.lib")  // Link Winsock library for networking on Windows
#else
    #include <arpa/inet.h>  // Library for internet address manipulation (used in Unix-based systems)
#endif

// Message type constants: Define different request types for communication
#define REGISTER_REQUEST 0x00                     // Register a client to monitor a flight
#define QUERY_FLIGHT_ID_REQUEST 0x01               // Request to query flight based on flight ID
#define QUERY_FLIGHT_INFO_REQUEST 0x02             // Request detailed flight information
#define MAKE_SEAT_RESERVATION_REQUEST 0x03         // Request to make a seat reservation
#define QUERY_BAGGAGE_AVAILABILITY_REQUEST 0x04    // Request baggage availability for a flight
#define ADD_BAGGAGE_REQUEST 0x05                   // Request to add baggage to a flight

// Structure to represent a general communication message
typedef struct {
    uint8_t message_type;     // Type of message, defined by constants like REGISTER_REQUEST
    uint32_t request_id;      // Unique request ID for tracking requests
    uint32_t data_length;     // Length of the data field in the message
    uint8_t* data;            // Pointer to the actual data (can be flight info, baggage info, etc.)
} Message;

// Function declarations for handling different structures and messages

/**
 * @brief Creates a DepartureTime structure.
 * @param year The year of departure.
 * @param month The month of departure.
 * @param day The day of departure.
 * @param hour The hour of departure.
 * @param minute The minute of departure.
 * @return A pointer to the dynamically created DepartureTime structure.
 */
DepartureTime* create_departure_time(int year, int month, int day, int hour, int minute);

/**
 * @brief Frees memory allocated for a DepartureTime structure.
 * @param departure_time Pointer to the DepartureTime structure to free.
 */
void free_departure_time(DepartureTime* departure_time);

/**
 * @brief Creates a Flight structure.
 * @param flight_id The unique ID of the flight.
 * @param source The departure location.
 * @param destination The arrival location.
 * @param departure_time A pointer to a DepartureTime structure for the flight.
 * @param airfare The cost of the flight.
 * @param seat_availability The number of available seats.
 * @param baggage_availability The number of available baggage spots.
 * @return A pointer to the dynamically created Flight structure.
 */
Flight* create_flight(int flight_id, const char* source, const char* destination,
                      DepartureTime* departure_time, float airfare,
                      int seat_availability, int baggage_availability);

/**
 * @brief Frees memory allocated for a Flight structure.
 * @param flight Pointer to the Flight structure to free.
 */
void free_flight(Flight* flight);

/**
 * @brief Creates a Message structure to send flight-related data.
 * @param message_type Type of message (e.g., REGISTER_REQUEST).
 * @param request_id Unique ID for the request.
 * @param flight_data Pointer to a Flight structure containing flight details.
 * @return A pointer to the dynamically created Message structure.
 */
Message* create_message(uint8_t message_type, uint32_t request_id, Flight* flight_data);

/**
 * @brief Frees memory allocated for a Message structure.
 * @param message Pointer to the Message structure to free.
 */
void free_message(Message* message);

/**
 * @brief Converts a Message structure into a byte array for network transmission (serialization).
 * @param message Pointer to the Message structure to serialize.
 * @param out_length Pointer to store the length of the serialized byte array.
 * @return A pointer to the serialized byte array (in network format).
 */
uint8_t* marshal_message(const Message* message, uint32_t* out_length);

/**
 * @brief Converts a byte array into a Message structure (deserialization).
 * @param byte_array Pointer to the byte array received over the network.
 * @return A pointer to the dynamically created Message structure (after deserialization).
 */
Message* unmarshal_message(const uint8_t* byte_array);

/**
 * @brief Converts a Flight structure into a byte array for network transmission (serialization).
 * @param flight Pointer to the Flight structure to serialize.
 * @param out_length Pointer to store the length of the serialized byte array.
 * @return A pointer to the serialized byte array (in network format).
 */
uint8_t* marshal_flight(const Flight* flight, uint32_t* out_length);

/**
 * @brief Converts a byte array into a Flight structure (deserialization).
 * @param byte_array Pointer to the byte array containing flight data.
 * @param offset Pointer to track the current read position in the byte array.
 * @return A pointer to the dynamically created Flight structure (after deserialization).
 */
Flight* unmarshal_flight(const uint8_t* byte_array, uint32_t* offset);

#endif // COMMUNICATION_H
