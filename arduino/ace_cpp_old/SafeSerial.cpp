#include "SafeSerial.h"

#include <avr/interrupt.h>

typedef struct {
  uint8_t buffer[SERIAL_BUFFER_SIZE];
  volatile uint16_t head;
  volatile uint16_t tail;
} ring_buffer;

static ring_buffer tx_buffer = {{0}, 0, 0};

SafeSerial::begin(uint32_t baud) {
  //enable double speed transmission
  *ucsrXa |= (1 << u2xX);

  uint16_t baudSetting = (F_CPU / 4 / baud - 1) / 2;
  // assign the baud_setting, a.k.a. ubbr (USART Baud Rate Register)
  *ubrrXh = baudSetting >> 8;
  *ubrrXl = baudSetting;
  
  // defaults to 8-bit, no parity, 1 stop bit
  *ucsrXb |= (1 << *exenX) | (1 << *txenX) | (1 << *rxcieX);
  *ucsrXb &= ~(1 << *udrieX) | (1 << *txenX) | (1 << *rxcieX);
}


//receive
ISR(USART0_RX_vect) {
  uint8_t c = UDR0;
  serial.put(c);
}

//data register empty
ISR(USART0_UDRE_vect) {
  if (tx_buffer.head == tx_buffer.tail) {
    //buffer empty, disable interruot
    UCSR0B &= ~(1 << UDRIE0);
  } else {
    uint8_t c = tx_buffer.buffer[tx_buffer.tail];
    tx_buffer.tail = (tx_buffer.tail + 1) % SERIAL_BUFFER_SIZE;
    UDR0 = c; //write next byte
  }
}

void SafeSerial::send(uint8_t byte) {
  uint16_t i = (tx_buffer.head + 1) % SERIAL_BUFFER_SIZE;
	
  // If the output buffer is full, there's nothing for it other than to 
  // wait for the interrupt handler to empty it a bit
if (i == tx_buffer.tail) return;
  //while (i == tx_buffer.tail) {};
	
  tx_buffer.buffer[tx_buffer.head] = byte;
  tx_buffer.head = i;
	
	//enable data register empty interrupt
	UCSR0B |= (1 << UDRIE0);
};
