#include <string.h>
#include <stdlib.h>
#include "databus.h"
#include "uart.h"

int databus_init(size_t buffer_length, uint8_t bus_id) {
    databus_t* init_bus = calloc(1, sizeof(databus_t));
    uint8_t* init_buffer = calloc(1, buffer_length);
    if (init_bus != NULL && init_buffer != NULL) {
        init_bus->id = bus_id;
        init_bus->buffer_length = buffer_length;
        init_bus->read_index = 0;
        init_bus->write_index = 0;
        init_bus->count = 0;
        init_bus->data_buffer = init_buffer;
        bus_list[bus_id] = init_bus;
    } else {
        free(init_bus);
        free(init_buffer);
        return -1;
    }
    return 0;
}

void databus_free(uint8_t bus_id) {
    databus_t* temp = bus_list[bus_id];
    if (temp) {
        free(temp->data_buffer);
        free(temp);
        bus_list[bus_id] = NULL;
    }
}

//Read 
uint8_t databus_read(uint8_t bus_id, uint8_t* buffer, size_t bytes) {
    databus_t* temp = bus_list[bus_id];
    if (!temp || temp->count == 0) return 0;
    size_t to_read = (bytes > temp->count) ? temp->count : bytes;
    for (size_t i = 0; i < to_read; ++i) {
        buffer[i] = temp->data_buffer[temp->read_index];
        temp->read_index = (temp->read_index + 1) % temp->buffer_length;
    }
    temp->count -= to_read;
    return (uint8_t)to_read;
}

uint8_t databus_write(uint8_t bus_id, uint8_t* buffer, size_t bytes) {
    databus_t* temp = bus_list[bus_id];
    if (!temp) return 0;
    size_t space_left = temp->buffer_length - temp->count;
    size_t to_write = (bytes > space_left) ? space_left : bytes;
    for (size_t i = 0; i < to_write; ++i) {
        temp->data_buffer[temp->write_index] = buffer[i];
        temp->write_index = (temp->write_index + 1) % temp->buffer_length;
    }
    temp->count += to_write;
    return (uint8_t)to_write;
}

