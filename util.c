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


void util_inc_bcd8(__xdata uint8_t *v) __naked
{
	(void)v;
	__asm
		movx	A, @DPTR
		add	A, #1
		da	A
		movx	@DPTR, A
		ret
	__endasm;
}

void util_inc_bcd16(__xdata uint16_t *v) __naked
{
	(void)v;
	__asm
		movx	A, @DPTR
		add	A, #1
		da	A
		movx	@DPTR, A
		jnc	0001$

		inc	DPTR
		movx	A, @DPTR
		addc	A, #0
		da	A
		movx	@DPTR, A
	0001$:
		ret
	__endasm;
}
