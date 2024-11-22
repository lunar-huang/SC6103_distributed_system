#include <stdint.h>  // Include for defining types like uint8_t and uint32_t
#include "communication.h"  // Include for the definitions related to message structures
#include <stdlib.h>  // Standard library functions
#include <string.h>  // Functions for memory manipulation
#include "server.h"  // Include for server-related structures like Flight and DepartureTime

// Unmarshal an integer from a byte array
// This function extracts an integer (4 bytes) from the byte array starting at the given offset.
int unmarshal_int(const uint8_t* byte_array, uint32_t* offset) {
    uint32_t network_value;  // Variable to store the value in network byte order
    // Copy 4 bytes from the byte array starting at the current offset into network_value
    memcpy(&network_value, byte_array + *offset, 4);
    *offset += 4;  // Increment the offset by 4 bytes to move past the integer
    return ntohl(network_value);  // Convert from network byte order to host byte order
}

// Unmarshal a float from a byte array
// This function extracts a float (4 bytes) from the byte array starting at the given offset.
float unmarshal_float(const uint8_t* byte_array, uint32_t* offset) {
    uint32_t network_value;  // Variable to store the bit representation of the float
    // Copy 4 bytes from the byte array starting at the current offset into network_value
    memcpy(&network_value, byte_array + *offset, 4);
    *offset += 4;  // Increment the offset by 4 bytes
    network_value = ntohl(network_value);  // Convert from network byte order to host byte order
    // Return the float by interpreting the bits in the integer as a float
    return *(float*)&network_value;
}

// Unmarshal a string from a byte array
// This function extracts a string from the byte array, which is preceded by its length.
char* unmarshal_string(const uint8_t* byte_array, uint32_t* offset) {
    uint32_t str_len = unmarshal_int(byte_array, offset);  // First, get the length of the string (4 bytes)
    char* str = malloc(str_len + 1);  // Allocate memory for the string (plus 1 for null-terminator)
    // Copy the string from the byte array into the allocated memory
    memcpy(str, byte_array + *offset, str_len);
    str[str_len] = '\0';  // Add the null-terminator at the end of the string
    *offset += str_len;  // Increment the offset by the length of the string
    return str;  // Return the unmarshaled string
}

// Unmarshal a DepartureTime structure from a byte array
// This function extracts a DepartureTime structure, which consists of 5 integers.
DepartureTime unmarshal_departure_time(const uint8_t* byte_array, uint32_t* offset) {
    DepartureTime time;  // Create a DepartureTime structure
    // Unmarshal each field of the DepartureTime structure (year, month, day, hour, minute)
    time.year = unmarshal_int(byte_array, offset);
    time.month = unmarshal_int(byte_array, offset);
    time.day = unmarshal_int(byte_array, offset);
    time.hour = unmarshal_int(byte_array, offset);
    time.minute = unmarshal_int(byte_array, offset);
    return time;  // Return the unmarshaled DepartureTime structure
}

// Unmarshal a Flight structure from a byte array
// This function extracts all the fields of a Flight structure from the byte array.
Flight* unmarshal_flight(const uint8_t* byte_array, uint32_t* offset) {
    Flight* flight = malloc(sizeof(Flight));  // Allocate memory for the Flight structure
    flight->flight_id = unmarshal_int(byte_array, offset);  // Unmarshal the flight_id (4 bytes)
    flight->source_place = unmarshal_string(byte_array, offset);  // Unmarshal the source_place string
    flight->destination_place = unmarshal_string(byte_array, offset);  // Unmarshal the destination_place string
    
    // Unmarshal the DepartureTime structure
    flight->departure_time = unmarshal_departure_time(byte_array, offset);
    
    flight->airfare = unmarshal_float(byte_array, offset);  // Unmarshal the airfare (float)
    flight->seat_availability = unmarshal_int(byte_array, offset);  // Unmarshal the seat_availability (int)
    flight->baggage_availability = unmarshal_int(byte_array, offset);  // Unmarshal the baggage_availability (int)
    
    return flight;  // Return the unmarshaled Flight structure
}
