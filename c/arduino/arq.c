#include "arq.h"

#include <stdlib.h>
/* Expected frame data for ARQ
 * 
 * +-----+-----+------+
 * | seq | cmd | data |
 * +-----+-----+------+
 * 
 * seq: sequence number of frame (1 byte)
 * cmd: command id of frame (1 byte), currently either ACK or DATA
 * data: actual data of frame (arbitraty length, respecting frame limitations)
 * 
 * Note: frame checking, headers and footers are not handled by arq, see 'framing' instead
 *
*/

#define MAX_RESENDS 5 //number of resends before a timeout is generated
#define MAX_SEQ 255 //maximum sequence number of frames
#define DATA 0x05
#define ACK 0x06

#define SEQ_INDEX 0
#define CMD_INDEX 1
#define MESSAGE_OFFSET 2
#define MAX_MESSAGE_SIZE MAX_FRAME_SIZE - MESSAGE_OFFSET


typedef enum {
  RECEIVED_FRAME,
  TIMEOUT
} arq_event;

static void send_ack(arq* a, uint8_t seq) {
  uint8_t data[] = {seq, ACK};
  a->sender(2, data);
}

static void process_event(arq* a, arq_event event, int16_t data_size, uint8_t* data) {
  if (event == RECEIVED_FRAME) {
    uint8_t seq = data[SEQ_INDEX];
    uint8_t cmd = data[CMD_INDEX];
    uint8_t* message = &(data[MESSAGE_OFFSET]);
    int16_t message_size = data_size - MESSAGE_OFFSET;
    
    if (!a->awaiting_ack) { //ready to receive
      if (cmd == DATA) {
        if (a->last_received_seq != seq) {
          a->last_received_seq = seq;
          a->event_handler(RECEIVED, message_size, message);
        }
        send_ack(a, seq);
      }
      //ignore case in which an ack is received even though none is awaited
    } else { //awaiting ack
      a->awaiting_ack = false;
       a->stop_timer();
      
      if (cmd == ACK && seq == a->last_sent_buffer[SEQ_INDEX]) {
        a->event_handler(SEND_SUCCESS, a->last_sent_size - MESSAGE_OFFSET, &(a->last_sent_buffer[MESSAGE_OFFSET]));
      }
    }
    
  } else if (event == TIMEOUT) {
    if (a->resends > MAX_RESENDS) {
      a->awaiting_ack = false;
      a->stop_timer();
      a->event_handler(NO_ACK, a->last_sent_size - MESSAGE_OFFSET, &(a->last_sent_buffer[MESSAGE_OFFSET]));
    } else {
      a->resends += 1;
      a->sender(a->last_sent_size, a->last_sent_buffer);
      a->awaiting_ack = true;
      a->start_timer();
    } 
  }
}

void receive_frame(arq* a, int16_t size, uint8_t* data) {
  process_event(a, RECEIVED_FRAME, size, data);
}

void timeout(arq* a) {
  process_event(a, TIMEOUT, 0, NULL);
}

void send_message(arq* a, int16_t size, const uint8_t * const message) {
  if (a->awaiting_ack) {
    a->event_handler(BUSY, size, message);
    return;
  }
  
  if (size > MAX_MESSAGE_SIZE) {
    a->event_handler(SIZE_ERROR, size, message);
    return;
  }
    
  
  a->last_sent_buffer[SEQ_INDEX] += 1; //increment seq
  a->last_sent_buffer[CMD_INDEX] = DATA;
  int i;
  for (i = 0; i < size; ++i) {
    a->last_sent_buffer[i + MESSAGE_OFFSET] = message[i];
  }
  
  a->last_sent_size = MESSAGE_OFFSET + size;
  
  a->sender(a->last_sent_size, a->last_sent_buffer);
  a->awaiting_ack = true;
  a->resends = 0;
  a->start_timer();
}

void init_arq(arq* a) {
  a->last_sent_size = 0;
  a->last_received_seq = 0;
  a->resends = 0;
  a->awaiting_ack = false;
}


