#include "physical.h"
#include "link.h"

#include <avr/interrupt.h>

typedef struct {
  uint8_t buffer[SERIAL_BUFFER_SIZE];
  volatile uint16_t head;
  volatile uint16_t tail;
} ring_buffer;

static ring_buffer tx_buffer = {{0}, 0, 0};


void init_physical_layer(uint32_t baud) {
  //enable double speed transmission
  UCSR0A |= (1 << U2X0);

  uint16_t baud_setting = (F_CPU / 4 / baud - 1) / 2;
  // assign the baud_setting, a.k.a. ubbr (USART Baud Rate Register)
  UBRR0H = baud_setting >> 8;
  UBRR0L = baud_setting;
  
  // defaults to 8-bit, no parity, 1 stop bit
  UCSR0B |= (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0);
  UCSR0B &= ~(1 << UDRIE0) | (1 << TXEN0) | (1 << RXCIE0);
}


//receive
ISR(USART0_RX_vect) {
  uint8_t c = UDR0;
  from_physical_layer(c);
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

void to_physical_layer(uint8_t byte) {
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
