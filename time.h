#ifndef TIME_H
#define TIME_H

#include <stdint.h>

typedef struct {
	uint8_t tick;
	uint8_t sec_bcd;
	uint8_t min_bcd;
	uint8_t hour_bcd;
	uint8_t day_bcd;
	uint8_t month_bcd;
	uint16_t year_bcd;
} time_t;

void time_init();
void time_isr();
void time_get(time_t *time);

#endif
