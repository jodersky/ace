#include "Framer.h"

Framer::Framer():
  state(WAITING),
  position(-1),
  checksum(0) {
}

void Framer::reset() {
  position = -1;
  checksum = 0x00;
}

void Framer::accept(uint8_t byte) {
  
  //if a new byte would cause an overflow, restart frame
  //note that this should not happen if both communicating parties have defined the same maximum frame length
  if (position >= MAX_FRAME_SIZE) {
    reset();
    return;
  }
  
  if (position != -1) { // i.e. a previous byte exists
    frameBuffer[position] = staged; 
    checksum = checksum ^ staged;
  }
  
  position += 1;
  staged = byte;
  
}

void Framer::put(uint8_t byte) {
  
  switch(state) {
    case ESCAPING:
      accept(byte);
      state = RECEIVING;
      break;
      
    case WAITING:
      if (byte == START) {
        reset();
        state = RECEIVING;
      }
      break;
      
    case RECEIVING:
      switch(byte) {
        case ESCAPE:
          state = ESCAPING;
          break;
        case START:
          reset();
          break;
        case STOP:
          state = WAITING;
          if (staged == checksum) { //at this point, staged byte is the checksum sent in the frame (last byte of frame)
            onFrame(position, frameBuffer);
          }
          break;
          
        default:
          accept(byte);
          break;
      }
  }
}

  

void Framer::send(uint16_t length, const uint8_t * const data) {
  uint8_t checksum = 0x00;
  uint8_t byte;
  
  onSendByte(START);
  for (int i = 0; i < length; ++i) {
    byte = data[i];
    if (isCommandByte(byte))
      onSendByte(ESCAPE);
      checksum = checksum ^ byte;
      isCommandByte(byte);
  }
  if (isCommandByte(checksum)) {
    onSendByte(ESCAPE);
  }
  onSendByte(checksum);
  onSendByte(STOP);
};


 //~ cli();
  //~ TCCR1A = 0; // set entire TCCR1A register to 0
  //~ TCCR1B = 0; // same for TCCR1B
  //~ // turn on CTC mode:
  //~ TCCR1B |= (1 << WGM12);
  //~ // set CS31 for 64 prescaler
  //~ TCCR1B |= (1 << CS11);
  //~ // set compare match register to desired timer count:
  //~ OCR1A = F_CPU / 1000 / 64; // should be 250 for F_CPU=16Mhz and f = 1000Hz
  //~ sei();
