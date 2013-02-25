#ifndef FRAMER_H
#define FRAMER_H

#include <inttypes.h>

#define MAX_PACKET_SIZE 64
#define MAX_FRAME_SIZE MAX_PACKET_SIZE + 3

class Framer {
private:
  
  enum LinkState {
    WAITING,
    RECEIVING,
    ESCAPING
  };
  
  static const uint8_t ESCAPE = 0x02;
  static const uint8_t START = 0x03;
  static const uint8_t STOP = 0x10;
  
  LinkState state; //current state
  uint8_t frameBuffer[MAX_FRAME_SIZE]; //data of current frame
  uint8_t staged; //previous byte read, not defined when position == -1
  
  int16_t position; //position of next byte to be read into frame buffer, can also be -1 if no byte has yet been staged
  uint8_t checksum;
  
  
  inline void reset(); //reset current frame
  inline void accept(uint8_t byte); //reads a data byte (not a command byte) and takes an appropraiet action by modifying internal state
  inline static bool isCommandByte(uint8_t byte) {return (byte == START || byte == STOP || byte == ESCAPE);}
  
  
protected:

  Framer();  
  void put(uint8_t byte);
  void send(uint16_t length, const uint8_t * const data);
  
  /* Called when a byte is to be sent.
   * @param byte the byte to be sent */
  virtual void onSendByte(uint8_t byte) = 0;
  
  /* Called when a valid frame is received. */
  virtual void onFrame(uint16_t length, uint8_t *data) = 0;
  
};


#endif /* FRAMER_H */
