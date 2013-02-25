#ifndef LINK_H
#define LINK_H

#include <inttypes.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_PACKET_SIZE 256

typedef struct {
  uint8_t* data;
  uint16_t length;
} packet;

void to_link_layer(packet* s); //implemented in link
void from_link_layer(packet* r); //should be implemented in transport

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* LINK_H */
