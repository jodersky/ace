#ifndef FRAMING_H
#define FRAMING_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <inttypes.h>

#define MAX_FRAME_SIZE 128
#define FRAME_ESCAPE 0x02
#define FRAME_START 0x03
#define FRAME_STOP 0x10

typedef enum {
  WAITING,
  RECEIVING,
  ESCAPING
} link_state;

typedef void (*receive_frame_function)(int16_t size, uint8_t* data);
typedef void (*send_byte_function)(uint8_t byte);


typedef struct {
  link_state state; // current state
  uint8_t buffer[MAX_FRAME_SIZE]; // in data buffer
  uint8_t staged; // previously read byte, will be put in buffer after next read
  int16_t position; //current position of next byte to be read in buffer
  uint8_t checksum; //current value of calculated checksum
  
  receive_frame_function receiver;
  send_byte_function sender;
  
} framer;

void init_framer(framer* f);
void receive_byte(framer* f, uint8_t byte);
void send_frame(framer* f, int16_t size, const uint8_t * const data_out);


#ifdef __cplusplus
} // extern "C"
#endif

#endif /* FRAMING_H */
