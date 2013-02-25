#ifndef SAFESERIAL_H
#define SAFESERIAL_H

#include <inttypes.h>
#include "Framer.h"

//#define MAX_PACKET_SIZE 64
//#define MAX_FRAME_SIZE MAX_PACKET_SIZE + 3

#define SERIAL_BUFFER_SIZE 64

class SafeSerial: public Framer {
private:
  void sendByte(uint8_t byte);
  
  
protected:
  /* virtual */ void onSendByte(uint8_t byte) {};
  /* virtual */ void onFrame(uint16_t length, uint8_t *data) {};
  
};

#endif /* SAFESERIAL_H */
