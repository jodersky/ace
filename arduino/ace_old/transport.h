#ifndef TRANSPORT_H
#define TRANSPORT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <inttypes.h>

typedef struct {
  uint8_t* data;
  uint16_t length;
} message;

typedef enum {
  RECEIVED = 0,
  SEND_SUCCESS = 1,
  SEND_ERROR = 2,
  NO_ACK = 3,
  BAD_ACK = 4,
  BUSY = 5
} transport_code;

void init_transport_layer(uint32_t timeout_millis);
void to_transport_layer(message* s); //implemented in transport
void from_transport_layer(transport_code code, message* r); //should be implemented in application

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* TRANSPORT_H */
