#ifndef SIM_H
#define SIM_H

#include <string.h>
#include <stdint.h>

#if SIMULATION

int8_t sim_detect(void);
void sim_stop(void);
void sim_putc(char c);
void sim_puts(__code const char *str);
// print a single hex digit
void sim_putx(uint8_t c);
// print various types
void sim_put_uint8(uint8_t c);
void sim_put_uint16(uint16_t c);
void sim_put_uint32(uint32_t c);
void sim_put_intptr(intptr_t c);

#else // !SIMULATION

#define sim_detect() ((uint8_t)0==0)
#define sim_stop() do { } while (0)
#define sim_putc(X) do { } while (0)
#define sim_puts(X) do { } while (0)
#define sim_putx(X) do { } while (0)
#define sim_put_uint8(X) do { } while (0)
#define sim_put_uint16(X) do { } while (0)
#define sim_put_uint32(X) do { } while (0)
#define sim_put_intptr(X) do { } while (0)

#endif
#endif
