#include "ace.h"

#include <avr/interrupt.h>


typedef struct {
  uint8_t buffer[SERIAL_BUFFER_SIZE];
  volatile uint16_t head;
  volatile uint16_t tail;
} ring_buffer;

static ring_buffer tx_buffer0 = {{0}, 0, 0};
static framer framer0;
static arq arq0;
static uint32_t max_ticks0;
static uint32_t ticks0;

//initialize UART
static void init_s0(uint32_t baud) {
  UCSR0A |= (1 << U2X0);   //enable double speed transmission
  uint16_t baud_setting = (F_CPU / 4 / baud - 1) / 2;
  UBRR0H = baud_setting >> 8;
  UBRR0L = baud_setting;
  UCSR0B |= (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0);   // defaults to 8-bit, no parity, 1 stop bit
  UCSR0B &= ~(1 << UDRIE0) | (1 << TXEN0) | (1 << RXCIE0);
}

static void send_to_s0(uint8_t byte) {
  uint16_t i = (tx_buffer0.head + 1) % SERIAL_BUFFER_SIZE;
  // If the output buffer is full, there's nothing for it other than to 
  // wait for the interrupt handler to empty it a bit
  if (i == tx_buffer0.tail) return;
  //while (i == tx_buffer0.tail) {};
  tx_buffer0.buffer[tx_buffer0.head] = byte;
  tx_buffer0.head = i;
	//enable data register empty interrupt
	UCSR0B |= (1 << UDRIE0);
}

static void send_to_framer0(int16_t size, const uint8_t* const data) {
  send_frame(&framer0, size, data);
}

static void receive_from_framer0(int16_t size, uint8_t* data) {
  receive_frame(&arq0, size, data);
}

//called when byte is received
ISR(USART0_RX_vect) {
  uint8_t c = UDR0;
  receive_byte(&framer0, c);
}

//called when data register is empty
ISR(USART0_UDRE_vect) {
  if (tx_buffer0.head == tx_buffer0.tail) {
    UCSR0B &= ~(1 << UDRIE0); //buffer empty, disable interruot
  } else {
    uint8_t c = tx_buffer0.buffer[tx_buffer0.tail];
    tx_buffer0.tail = (tx_buffer0.tail + 1) % SERIAL_BUFFER_SIZE;
    UDR0 = c; //write next byte
  }
}

//initialize timer
static void init_t0(uint32_t ticks) {
  cli();
  TCCR1A = 0;
  TCCR1B = 0;
  TCCR1B |= (1 << WGM12); // turn on CTC mode:
  TCCR1B |= (1 << CS11); // set CS31 for 64 prescaler
  // set compare match register to desired timer count:
  OCR1A = F_CPU / 1000 / 64; // should be 250 for F_CPU=16Mhz and f = 1000Hz
  sei();
  //the timer should now interrupt every ms
  max_ticks0 = ticks;
  ticks0 = 0;
}

inline static void start_t0() {
  TIMSK1 |= (1 << OCIE1A);
}

inline static void stop_t0() {
  TIMSK1 &= ~(1 << OCIE1A);
}

ISR(TIMER1_COMPA_vect) {
  ticks0 += 1;
  if (ticks0 > max_ticks0) {
    timeout(&arq0);
    ticks0 = 0;
  }
} 

void init_ace0(uint32_t baud, uint32_t extra_time) {
  init_s0(baud);
  init_framer(&framer0);
  init_arq(&arq0);
  
  framer0.sender = send_to_s0;
  arq0.sender = send_to_framer0;
  
  framer0.receiver = receive_from_framer0;
  arq0.event_handler = ace_event0;
  
  uint32_t bits = (MAX_FRAME_SIZE + 3) * 8 * 2;
  uint32_t tx_time = bits * 1000 / baud; // transmission time in milliseconds
  init_t0(tx_time + extra_time);
  arq0.start_timer = start_t0;
  arq0.stop_timer = stop_t0;
}

void ace_send0(uint8_t size, const uint8_t* const message) {
  send_message(&arq0, size, message);
}
