#ifndef DATABUS_H
#define DATABUS_H

#include <stdint.h>
#include <stddef.h> // For size_t

#define BUFFER_SIZE 255
enum bus_id{
  SERIAL = 0x00,
  ADC = 0x01,
  };//max is 255(uint8_t id)

typedef struct {
    uint8_t id;
    size_t buffer_length;
    size_t read_index;   // New: index to read from
    size_t write_index;  // New: index to write to
    size_t count;        // New: number of bytes in buffer
    uint8_t* data_buffer;
} databus_t;

static databus_t* bus_list[20];

// Initialize a databus instance
// Returns > 0 on success
int databus_init(size_t buffer_length, uint8_t bus_id) ;

// Free a databus instance
void databus_free(uint8_t bus_id);

// Read data from a databus instance into a buffer
// Returns the number of bytes read
uint8_t databus_read(uint8_t bus_id, uint8_t* buffer, size_t length);

// Write data from a buffer to a databus instance
// Returns the number of bytes written (or status code)
uint8_t databus_write(uint8_t bus_id, uint8_t* buffer, size_t length);

#endif // DATABUS_H