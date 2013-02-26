#ifndef ACE_H
#define ACE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <inttypes.h>
#include "framing.h"
#include "arq.h"

#define SERIAL_BUFFER_SIZE 64

/**
* A reliable, reactive protocol implementation. Note that the default physical channel used is serial 0 and timer 1 on an Arduino.
* This may be changed in the .c file.
*/

/** Initialize ACE for serial 0. */
void init_ace0(uint32_t baud, uint32_t extra_time);

/** Send a message over serial 0. */
void ace_send0(uint8_t size, const uint8_t* const message);

/** Called on message event for serial 0.
* @param event event type, see arq.h for possible values
* @param size size of message (if given)
* @param message message if given or NULL
*/
void ace_event0(message_event event, int16_t size, const uint8_t* const message);

#define init_ace init_ace0
#define ace_send ace_send0
#define ace_event ace_event0

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* ACE_H */
