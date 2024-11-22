#include <stdint.h>  // Add this header file to define uint8_t and uint32_t
#include "communication.h"  // Include custom communication header file
#include <stdlib.h>  // Standard library for memory management functions
#include <string.h>  // Include string manipulation functions
#include "server.h"  // Include server definitions

// Marshal an integer to a byte array (4 bytes)
uint8_t* marshal_int(int value) {
    uint32_t network_value = htonl(value);  // Convert integer from host to network byte order (big-endian)
    uint8_t* buffer = malloc(4);  // Allocate 4 bytes for the integer
    memcpy(buffer, &network_value, 4);  // Copy the network value into the allocated buffer
    return buffer;  // Return the marshaled byte array
}

// Marshal a float to a byte array (4 bytes)
uint8_t* marshal_float(float value) {
    uint32_t* int_rep = (uint32_t*)&value;  // Treat the float as an integer bit pattern
    uint32_t network_value = htonl(*int_rep);  // Convert the bit pattern from host to network byte order
    uint8_t* buffer = malloc(4);  // Allocate 4 bytes for the float value
    memcpy(buffer, &network_value, 4);  // Copy the network value into the buffer
    return buffer;  // Return the marshaled byte array
}

// Marshal a string with length prefix
uint8_t* marshal_string(const char* str, uint32_t* out_length) {
    uint32_t str_len = strlen(str);  // Get the length of the string
    *out_length = 4 + str_len;  // Total size is 4 bytes for the length prefix plus the string length
    uint8_t* buffer = malloc(*out_length);  // Allocate memory for the string and its length prefix

    uint32_t network_len = htonl(str_len);  // Convert string length to network byte order
    memcpy(buffer, &network_len, 4);  // Copy the length prefix to the buffer
    memcpy(buffer + 4, str, str_len);  // Copy the actual string data after the length prefix
    return buffer;  // Return the marshaled string with length prefix
}

// Marshal a DepartureTime structure (containing year, month, day, hour, and minute)
uint8_t* marshal_departure_time(const DepartureTime* departure, uint32_t* out_length) {
    *out_length = 5 * 4;  // Allocate 4 bytes for each of the 5 integer fields
    uint8_t* buffer = malloc(*out_length);  // Allocate memory for the departure time structure

    // Marshal and copy each field into the buffer
    memcpy(buffer, marshal_int(departure->year), 4);
    memcpy(buffer + 4, marshal_int(departure->month), 4);
    memcpy(buffer + 8, marshal_int(departure->day), 4);
    memcpy(buffer + 12, marshal_int(departure->hour), 4);
    memcpy(buffer + 16, marshal_int(departure->minute), 4);

    return buffer;  // Return the marshaled departure time structure
}

// Marshal a Flight structure
uint8_t* marshal_flight(const Flight* flight, uint32_t* out_length) {
    uint32_t source_len, dest_len, time_len;

    // Marshal the source and destination places and departure time
    uint8_t* source = marshal_string(flight->source_place, &source_len);
    uint8_t* dest = marshal_string(flight->destination_place, &dest_len);
    uint8_t* time = marshal_departure_time(&(flight->departure_time), &time_len);

    // Calculate the total size: flight_id (4 bytes), source, destination, departure time, airfare (4 bytes), seat availability (4 bytes), baggage availability (4 bytes)
    *out_length = 4 + source_len + dest_len + time_len + 4 + 4 + 4;
    uint8_t* buffer = malloc(*out_length);  // Allocate memory for the entire flight structure

    // Copy the marshaled fields into the buffer, updating the offset for each field
    uint32_t offset = 0;
    memcpy(buffer + offset, marshal_int(flight->flight_id), 4); offset += 4;
    memcpy(buffer + offset, source, source_len); offset += source_len;
    memcpy(buffer + offset, dest, dest_len); offset += dest_len;
    memcpy(buffer + offset, time, time_len); offset += time_len;
    memcpy(buffer + offset, marshal_float(flight->airfare), 4); offset += 4;
    memcpy(buffer + offset, marshal_int(flight->seat_availability), 4); offset += 4;
    memcpy(buffer + offset, marshal_int(flight->baggage_availability), 4);

    // Free dynamically allocated memory for individual marshaled fields
    free(source);
    free(dest);
    free(time);

    return buffer;  // Return the marshaled flight structure
}

// Marshal a Message structure
uint8_t* marshal_message(const Message* message, uint32_t* out_length) {
    *out_length = 1 + 4 + 4 + message->data_length;  // Calculate total size: message type (1 byte), request_id (4 bytes), data_length (4 bytes), and the data itself
    uint8_t* buffer = malloc(*out_length);  // Allocate memory for the entire message

    // Copy message fields into the buffer
    buffer[0] = message->message_type;  // 1 byte for message type
    memcpy(buffer + 1, marshal_int(message->request_id), 4);  // 4 bytes for request ID
    memcpy(buffer + 5, marshal_int(message->data_length), 4);  // 4 bytes for data length
    memcpy(buffer + 9, message->data, message->data_length);  // Copy the actual data

    return buffer;  // Return the marshaled message
}
