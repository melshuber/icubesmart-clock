#include <stdint.h>

#include "sim.h"

__xdata __at(0xffff) uint8_t sif;

enum sif_command {
  DETECT_SIGN	        = '!',	// answer to detect command
  SIFCM_DETECT		= '_',	// command used to detect the interface
  SIFCM_COMMANDS	= 'i',	// get info about commands
  SIFCM_IFVER		= 'v',	// interface version
  SIFCM_SIMVER		= 'V',	// simulator version
  SIFCM_IFRESET		= '@',	// reset the interface
  SIFCM_CMDINFO		= 'I',	// info about a command
  SIFCM_CMDHELP		= 'h',	// help about a command
  SIFCM_STOP		= 's',	// stop simulation
  SIFCM_PRINT		= 'p',	// print character
  SIFCM_FIN_CHECK	= 'f',	// check input file for input
  SIFCM_READ		= 'r',	// read from input file
  SIFCM_WRITE		= 'w',	// write to output file
};

int8_t sim_detect(void) __critical
{
	sif = SIFCM_DETECT;
	return sif == DETECT_SIGN;
}

void sim_stop(void)
{
	sim_puts("stopping\n");
	sif = SIFCM_STOP;
}

void sim_putc(char c) __critical
{
	sif = SIFCM_PRINT;
	sif = (uint8_t)c;
}

void sim_puts(__code const char *str)
{
	while (*str) {
		sim_putc(*str);
		str++;
	}
}

void sim_putx(uint8_t c) __critical
{
	sif = SIFCM_PRINT;

	sif = (c < 10) ?
		('0' + c) :
		('A' + c - 10);
}

void sim_put_uint8(uint8_t c)
{
	sim_putx((c >> 4) & 0xf);
	sim_putx(c & 0xf);
}

void sim_put_uint16(uint16_t c)
{
	sim_putx((c >> 12) & 0xf);
	sim_putx((c >> 8) & 0xf);
	sim_putx((c >> 4) & 0xf);
	sim_putx(c & 0xf);
}

void sim_put_uint32(uint32_t c)
{
	sim_putx((c >> 28) & 0xf);
	sim_putx((c >> 24) & 0xf);
	sim_putx((c >> 20) & 0xf);
	sim_putx((c >> 16) & 0xf);
	sim_putx((c >> 12) & 0xf);
	sim_putx((c >> 8) & 0xf);
	sim_putx((c >> 4) & 0xf);
	sim_putx(c & 0xf);
}

void sim_put_intptr(intptr_t c)
{
	sim_put_uint32(c);
}
