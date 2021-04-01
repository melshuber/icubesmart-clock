#ifndef CPU_H
#define CPU_H

/* we do not use stc89.h, since it does not facilitate proper external
 * definitions. */
// #include <stc89.h>

/* GPIOs to 3d Matrix Display System */
extern __sfr P0;
extern __sfr P1;
extern __sfr P2;

/* Timer 0/1 Registers */
extern __sfr TCON;
extern __sbit IT0;
extern __sbit IE0;
extern __sbit IT1;
extern __sbit IE1;
extern __sbit TR0;
extern __sbit TF0;
extern __sbit TR1;
extern __sbit TF1;
extern __sfr TMOD;
extern __sfr TL0;
extern __sfr TL1;
extern __sfr TH0;
extern __sfr TH1;

/* Timer 2 Registers */
extern __sfr T2CON;
extern __sfr T2MOD;
extern __sfr RCAP2L;
extern __sfr RCAP2H;
extern __sfr TL2;
extern __sfr TH2;

/* Interrupt Registers */
extern __sfr IE;
extern __sbit EX0;
extern __sbit ET0;
extern __sbit EX1;
extern __sbit ET1;
extern __sbit ES;
extern __sbit EA;

#define NOP()						\
	__asm						\
		nop					\
	__endasm
		

#endif
