#ifdef DEBUG_UTIL
#define DEBUG_MODULE util
#endif
#include "debug.h"

#include "util.h"

uint8_t util_dec_to_bcd8(char *c) __reentrant
{
	uint8_t tmp = 0;
	if ((*c >= '0') && (*c <= '9')) {
		tmp |= (*c - '0');
	}
	c++;
	tmp <<= 4;
	if ((*c >= '0') && (*c <= '9')) {
		tmp |= (*c - '0');
	}
	return tmp;
}

uint16_t util_dec_to_bcd16(char *c) __reentrant
{
	uint8_t hi = util_dec_to_bcd8(c);
	uint8_t lo = util_dec_to_bcd8(c + 2);
	return (((uint16_t)hi) << 8) | ((uint16_t)lo);
}
