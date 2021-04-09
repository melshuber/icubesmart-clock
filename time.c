#ifdef DEBUG_MAIN
#define DEBUG_MODULE main
#endif
#include "debug.h"

#include "board.h"
#include "time.h"

#include <stdint.h>
#include <string.h>
#include <stdio.h>

static __xdata time_t _time;
static volatile __xdata uint16_t _time_ccap_val;

void time_init() __critical
{
	/* Setup PCA in in timer mode */
	_time_ccap_val = TIME_TICKS;
	TIME_CCAPL = TIME_TICKS & 0xff;
	TIME_CCAPH = (TIME_TICKS >> 8) & 0xff;
	TIME_PCA_PWM = 0;
	TIME_CCAPM = TIME_ECOM_val | TIME_MAT_val | TIME_ECCF_val;

	_time.tick = 0;
	_time.ticks = 0;
	_time.sec_bcd = 0x00;
	_time.min_bcd = 0x00;
	_time.hour_bcd = 0x00;
	_time.day_bcd = 0x01;
	_time.month_bcd = 0x01;
	_time.year_bcd = 0x2021;
}

void time_get(time_t *time)
{
	TIME_CCAPM = TIME_ECOM_val | TIME_MAT_val;

	memcpy(time, &_time, sizeof(time_t));

	TIME_CCAPM = TIME_ECOM_val | TIME_MAT_val | TIME_ECCF_val;
}

void time_set(const time_t *time) __critical
{
	_time.sec_bcd = time->sec_bcd;
	_time.min_bcd = time->min_bcd;
	_time.hour_bcd = time->hour_bcd;
	_time.day_bcd = time->day_bcd;
	_time.month_bcd = time->month_bcd;
	_time.year_bcd = time->year_bcd;
}

void _time_inc_bcd8(__xdata uint8_t *v) __naked
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

void _time_inc_bcd16(__xdata uint16_t *v) __naked
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

static uint8_t _time_days_in_month_bcd(__xdata time_t *time)
{
	const uint8_t _days_in_month[0x13] = {
		0,    /* 0x00: undef */
		0x31, /* 0x01: Jan */
		0x28, /* 0x02: Feb */
		0x31, /* 0x03: Mar */
		0x30, /* 0x04: Apr */
		0x31, /* 0x05: May */
		0x30, /* 0x06: Jun */
		0x31, /* 0x07: Jul */
		0x31, /* 0x08: Aug */
		0x30, /* 0x09: Sep */
		0,    /* 0x0a: undef */
		0,    /* 0x0b: undef */
		0,    /* 0x0c: undef */
		0,    /* 0x0d: undef */
		0,    /* 0x0e: undef */
		0,    /* 0x0f: undef */
		0x31, /* 0x10: Oct */
		0x30, /* 0x11: Nov */
		0x31, /* 0x12: Dez */
	};

	uint8_t month_bcd = time->month_bcd;
	uint8_t res = _days_in_month[month_bcd];

	/* No gap year adjustment for non-February */
	if (month_bcd != 0x02)
		return res;

	uint16_t year_bcd = time->year_bcd;

	/* No gap year adjustment for year xx00 */
	if ((year_bcd & 0xFF) == 0)
		return res;

	if ((year_bcd & 0x10) == 0) {
		/* years xx0x, xx2xx, ..*/

		if ((year_bcd % 4) != 0)
			return res;

		/* only xx04, xx08, xx20, xx24, .. are gap years */
		return 0x29;
	} else {
		/* years xx1x, xx3xx, ..*/

		if ((year_bcd % 4) != 2)
			return res;

		/* only xx12, xx16, xx32, xx36, .. are gap years */
		return 0x29;
	}
}

static void _time_do_tick()
{
	_time.ticks++;

        _time.tick++;
	if (_time.tick != TIME_TICK_HZ)
		return;
	_time.tick = 0;

	_time_inc_bcd8(&_time.sec_bcd);
	if (_time.sec_bcd != 0x60)
		return;
	_time.sec_bcd = 0x00;

	_time_inc_bcd8(&_time.min_bcd);
	if (_time.min_bcd != 0x60)
		return;
	_time.min_bcd = 0x00;

	_time_inc_bcd8(&_time.hour_bcd);
	if (_time.hour_bcd != 0x24)
		return;
	_time.hour_bcd = 0x00;

       _time_inc_bcd8(&_time.day_bcd);
       if (_time.day_bcd <= _time_days_in_month_bcd(&_time))
		return;
       _time.day_bcd = 0x01;

       _time_inc_bcd8(&_time.month_bcd);
       if (_time.month_bcd <= 12)
		return;
       _time.month_bcd = 0x01;

       _time_inc_bcd16(&_time.year_bcd);
}

void time_isr()
{
	uint16_t tmp = _time_ccap_val;
	tmp += TIME_TICKS;
	_time_ccap_val = tmp;
	TIME_CCAPL = tmp & 0xff;
	TIME_CCAPH = (tmp >> 8) & 0xff;

	_time_do_tick();
}
