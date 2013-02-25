#include "link.h"
#include "physical.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

typedef enum {
  WAITING,
  RECEIVING,
  ESCAPING
} link_state;

#define FRAME_ESCAPE 0x02
#define FRAME_START 0x03
#define FRAME_END 0x10


void from_physical_layer(uint8_t byte) {
  static link_state state = WAITING;
  static uint16_t position = 0; //read position
  static uint8_t packet_data_buffer[MAX_PACKET_SIZE];
  static packet received = {packet_data_buffer, 0};
  static uint8_t checksum = 0x00;
  
  static bool previous_exists = false;
  static uint8_t previous_byte;
  
  #define start_receiving() \
    position = 0; \
    previous_exists = false; \
    checksum = 0x00
  
  #define accept(byte) \
    if (position >= MAX_PACKET_SIZE) {start_receiving(); return;} \
    if (previous_exists) { \
      packet_data_buffer[position] = previous_byte; \
      position = position + 1; \
      checksum = checksum ^ previous_byte; \
      previous_byte = byte; \
    } else { \
      previous_byte = byte; \
      previous_exists = true; \
    }
    

  switch(state) {
    case ESCAPING:
      accept(byte);
      state = RECEIVING;
      break;
      
    case WAITING:
      if (byte == FRAME_START) {
        start_receiving();
        state = RECEIVING;
      }
      break;
    case RECEIVING:
      switch(byte) {
        case FRAME_ESCAPE:
          state = ESCAPING;
          break;
        case FRAME_START:
          start_receiving();
          break;
        case FRAME_END:
          state = WAITING;
          uint8_t sent_checksum = previous_byte;
          if (sent_checksum == checksum) {
            received.length = position;
            from_link_layer(&received);
          }
          
          break;
        default:
          accept(byte);
          break;
      }
  }
  
  #undef accept
  #undef start_receiving
}

void to_link_layer(packet* s) {
  uint8_t checksum = 0x00;
  uint8_t byte;
  int i;
  
  to_physical_layer(FRAME_START);
  for (i = 0; i < (s->length); ++i) {
    byte = s->data[i];
    if (byte == FRAME_START || byte == FRAME_END || byte == FRAME_ESCAPE)
      to_physical_layer(FRAME_ESCAPE);
    checksum = checksum ^ byte;
    to_physical_layer(byte);
  }
  if (checksum == FRAME_START || checksum == FRAME_END || checksum == FRAME_ESCAPE)
    to_physical_layer(FRAME_ESCAPE);
  to_physical_layer(checksum);
  to_physical_layer(FRAME_END);
};
