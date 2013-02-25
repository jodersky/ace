#ifndef PHYSICAL_H
#define PHYSICAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <inttypes.h>
#define SERIAL_BUFFER_SIZE 64

void init_physical_layer(uint32_t baud);
void to_physical_layer(uint8_t s); //implemented in physical
void from_physical_layer(uint8_t r); //should be implemented in link

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* PHYSICAL_H */
