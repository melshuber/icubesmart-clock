#ifndef UTIL_H
#define UTIL_H

#include <stdint.h>

uint8_t util_dec_to_bcd8(char *c) __reentrant;
uint16_t util_dec_to_bcd16(char *c) __reentrant;
void util_inc_bcd8(__xdata uint8_t *v);
void util_inc_bcd16(__xdata uint16_t *v);

#endif
