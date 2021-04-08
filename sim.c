#include <stdint.h>
#include <stdio.h>

#include "sim.h"

__sfr __at(0xff) sif;

enum sif_command {
  DETECT_SIGN		= '!',	// answer to detect command
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

void sim_putc(char c) __critical
{
	sif = SIFCM_PRINT;
	sif = (uint8_t)c;
}

void sim_stop(void)
{
	printf("stopping\n");
	sif = SIFCM_STOP;
}
