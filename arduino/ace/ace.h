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

void init_ace0(uint32_t baud, uint32_t extra_time);
void ace_send0(uint8_t size, const uint8_t* const message);
void ace_event0(message_event event, int16_t size, const uint8_t* const message);

#define init_ace init_ace0
#define ace_send ace_send0
#define ace_event ace_event0

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* ACE_H */
