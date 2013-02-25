#include "framing.h"

static inline bool is_cmd(uint8_t byte) {
  return (byte == FRAME_START || byte == FRAME_STOP || byte == FRAME_ESCAPE);
}

static inline void reset(framer* f) {
  f->position = -1;
  f->checksum = 0x00;
}

static inline void accept(framer* f, uint8_t byte) {
  
  //if a new byte would cause an overflow, restart frame
  //note that this should not happen if both communicating parties have defined the same maximum frame length
  if (f->position >= MAX_FRAME_SIZE) {
    reset(f);
    return;
  }
  
  if (f->position != -1) { // i.e. a previous byte exists
    f->buffer[f->position] = f->staged; 
    f->checksum = f->checksum ^ f->staged;
  }
  
  f->position += 1;
  f->staged = byte;
}

void receive_byte(framer* f, uint8_t byte) {
  
  switch(f->state) {
    case ESCAPING:
      accept(f, byte);
      f->state = RECEIVING;
      break;
      
    case WAITING:
      if (byte == FRAME_START) {
        reset(f);
        f->state = RECEIVING;
      }
      break;
      
    case RECEIVING:
      switch(byte) {
        case FRAME_ESCAPE:
          f->state = ESCAPING;
          break;
        case FRAME_START:
          reset(f);
          break;
        case FRAME_STOP:
          f->state = WAITING;
          if (f->staged == f->checksum) { //at this point, staged byte is the checksum sent in the frame (last byte of frame)
            f->receiver(f->position, f->buffer);
          }
          break;
          
        default:
          accept(f, byte);
          break;
      }
  }
}

void send_frame(framer* f, int16_t size, const uint8_t * const data_out) {
  uint8_t checksum = 0x00;
  uint8_t byte;
  send_byte_function sender = f->sender;
  
  sender(FRAME_START);
  int i;
  for (i = 0; i < size; ++i) {
    byte = data_out[i];
    if (is_cmd(byte)) sender(FRAME_ESCAPE);
    sender(byte);
    checksum = checksum ^ byte;
  }
  if (is_cmd(checksum)) sender(FRAME_ESCAPE);
  sender(checksum);
  sender(FRAME_STOP);
}

void init_framer(framer* f) {
  f->state = WAITING;
  reset(f);
}
