#ifndef SIM_H
#define SIM_H

#include <string.h>

#if SIMULATION

int8_t sim_detect(void);
void sim_stop(void);
void sim_putc(char c);
void sim_puts(__code const char *str);

#else // !SIMULATION

#define sim_detect() ((uint8_t)0==0)
#define sim_stop() do { } while (0)
#define sim_putc(X) do { } while (0)
#define sim_puts(X) do { } while (0)

#endif
#endif
