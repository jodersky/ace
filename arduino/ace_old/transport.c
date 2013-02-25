#include "transport.h"
#include "link.h"

#include <stdlib.h>
#include <stdbool.h>
#include <avr/interrupt.h>

//ms
#define EXTRA_TIME 200
#define MAX_RESENDS 5
#define MAX_SEQ 255

#define next_seq(k) if (k < MAX_SEQ) k = k + 1; else k = 0

#define DATA 0x05
#define ACK 0x06


static void start_sending(message* msg, bool resend);
static void stop_sending();


typedef enum {
  RECEIVED_PACKET,
  TIMEOUT
} event_kind;

typedef struct {
  event_kind kind;
  packet* payload;
} event;

static void disassemble_packet(packet* p, uint8_t* seq, uint8_t* command, uint8_t** msg, uint16_t* message_length) {
  if (seq != NULL) *seq = p->data[0];
  if (command != NULL) *command = p->data[1];
  if (msg != NULL) *msg = &(p->data[2]);
  if (message_length != NULL) *message_length = p->length - 2;
}

static void assemble_packet(packet* p, uint8_t seq, uint8_t command, uint8_t* msg, uint16_t message_length) {
  p->data[0] = seq;
  p->data[1] = command;
  int i;
  for (i = 0; i < message_length; ++i) {
    p->data[i+2] = msg[i];
  }
  p->length = message_length + 2;
}

static packet* ack(uint8_t seq) {
  static uint8_t ack_buffer[] = {0, ACK};
  static packet ack_packet = {ack_buffer, 2};
  ack_buffer[0] = seq;
  return &ack_packet;
}

static volatile bool awaiting_ack = false;
static volatile uint8_t last_sent_seq = 0;
static volatile uint8_t resends = 0;
static volatile uint8_t last_sent_buffer[MAX_PACKET_SIZE]; 
static volatile packet last_sent = {last_sent_buffer, 0};

static void process_event(event* e) {
  static uint8_t last_received_seq = 0;
  
  switch(e->kind) {
    case RECEIVED_PACKET: {
      uint8_t seq;
      uint8_t cmd;
      static message msg;
      disassemble_packet(e->payload, &seq, &cmd, &msg.data, &msg.length);
      
      if (!awaiting_ack) {
        if (cmd == DATA) {
          if (last_received_seq != seq) {
            last_received_seq = seq;
            from_transport_layer(RECEIVED, &msg);
          }
          to_link_layer(ack(seq));
        }
        //ignore case in which an ack is received even though none is awaited
 
      } else { //waiting for an ack
        stop_sending();
        disassemble_packet(&last_sent, NULL, NULL, &msg.data, &msg.length);
        if (cmd == ACK && seq == last_sent_seq) {
          from_transport_layer(SEND_SUCCESS, &msg);
        } else {
          from_transport_layer(BAD_ACK, &msg);
        }
      }      
    } break;
    case TIMEOUT:
      if (resends > MAX_RESENDS) {
        message msg;
        disassemble_packet(&last_sent, NULL, NULL, &msg.data, &msg.length);
        stop_sending();
        from_transport_layer(NO_ACK, &msg);
      } else {
        start_sending(NULL, true);
      }
      break;
  }
}

static void start_sending(message* msg, bool resend) {  
  if (resend) {
    resends = resends + 1;
  } else {
    next_seq(last_sent_seq);
    assemble_packet(&last_sent, last_sent_seq, DATA, msg->data, msg->length);
    resends = 0;
    TIMSK1 |= (1 << OCIE1A);
  }
  
  awaiting_ack = true;
  to_link_layer(&last_sent);
}

static void stop_sending() {
  TIMSK1 &= ~(1 << OCIE1A);
  awaiting_ack = false;
}

static volatile uint32_t ticks;
static volatile uint32_t max_ticks;

void init_transport_layer(uint32_t timeout_millis) {
  cli();
  TCCR1A = 0; // set entire TCCR1A register to 0
  TCCR1B = 0; // same for TCCR1B
  // turn on CTC mode:
  TCCR1B |= (1 << WGM12);
  // set CS31 for 64 prescaler
  TCCR1B |= (1 << CS11);
  // set compare match register to desired timer count:
  OCR1A = F_CPU / 1000 / 64; // should be 250 for F_CPU=16Mhz and f = 1000Hz
  sei();
  
  //the timer should now interrupt every ms
  max_ticks = timeout_millis;
}

ISR(TIMER1_COMPA_vect) {
  static event e = {TIMEOUT, NULL};
  
  ticks = ticks + 1;
  if (ticks > max_ticks) {
    process_event(&e);
    ticks = 0;
  }
} 

void from_link_layer(packet* r) {
 static event e = {RECEIVED_PACKET, NULL};
 e.payload = r;
 process_event(&e);
}

void to_transport_layer(message* s) {
  if (!awaiting_ack)
    start_sending(s, false);
  else
    from_transport_layer(BUSY, s);
}

