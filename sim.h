#ifndef SIM_H
#define SIM_H

#include <string.h>
#include <stdint.h>

int8_t sim_detect(void);
void sim_putc(char c);
void sim_stop(void);

#endif
