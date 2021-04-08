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

/* UART Registers */
extern __sfr SCON;
extern __sbit RI;
extern __sbit TI;
extern __sbit RB8;
extern __sbit TB8;
extern __sbit REN;
extern __sbit SM2;
extern __sbit SM1;
extern __sbit SM0;
extern __sfr SBUF;
extern __sfr BRT;

/* PCA Registers */
extern __sfr CCON;
extern __sbit CF;
extern __sbit CR;
extern __sbit CCF1;
extern __sbit CCF0;
#define CCF0_val   (1 << 0)
#define CCF1_val   (1 << 1)
#define CR_val     (1 << 6)
#define CF_val     (1 << 7)
extern __sfr CMOD;
#define ECF_val    (1 << 0)
#define CPS0_val   (1 << 1)
#define CPS1_val   (1 << 2)
#define CPS2_val   (1 << 3)
#define CPS_div12  (0 << 1)
#define CPS_div2   (1 << 1)
#define CPS_timer0 (2 << 1)
#define CPS_ext    (3 << 1)
#define CPS_div1   (4 << 1)
#define CPS_div4   (5 << 1)
#define CPS_div6   (6 << 1)
#define CPS_div8   (7 << 1)
#define CIDL_val   (1 << 4)
extern __sfr CCAPM0;
#define ECCF0_val  (1 << 0)
#define PWM0_val   (1 << 1)
#define TOG0_val   (1 << 2)
#define MAT0_val   (1 << 3)
#define CAPN0_val  (1 << 4)
#define CAPP0_val  (1 << 5)
#define ECOM0_val  (1 << 6)
extern __sfr CCAPM1;
#define ECCF1_val  (1 << 0)
#define PWM1_val   (1 << 1)
#define TOG1_val   (1 << 2)
#define MAT1_val   (1 << 3)
#define CAPN1_val  (1 << 4)
#define CAPP1_val  (1 << 5)
#define ECOM1_val  (1 << 6)
extern __sfr CL;
extern __sfr CH;
extern __sfr CCAP0L;
extern __sfr CCAP0H;
extern __sfr CCAP1L;
extern __sfr CCAP1H;
extern __sfr PCA_PWM0;
#define EPC0L_val (1 << 0)
#define EPC0H_val (1 << 1)
extern __sfr PCA_PWM1;
#define EPC1L_val (1 << 0)
#define EPC1H_val (1 << 1)

/* Interrupt Registers */
extern __sfr IE;
extern __sbit EX0;
extern __sbit ET0;
extern __sbit EX1;
extern __sbit ET1;
extern __sbit ES;
extern __sbit EA;

/* POWER Registers */
extern __sfr PCON;
#define PCON_SMOD0_val (1 << 6)
#define PCON_SMOD_val (1 << 7)

/* ISP Registers */
extern __sfr IAP_CONTR;

/* Other Registers */
extern __sfr AUXR;
#define S1BRS_val (1 << 0)
#define EXTRAM_val (1 << 1)
#define BRTx12_val (1 << 2)
#define S2SMOD_val (1 << 3)
#define BRTR_val (1 << 4)
#define UART_M0x6_val (1 << 5)
#define T1x12_val (1 << 6)
#define T0x12_val (1 << 7)
extern __sfr AUXR1;
#define DPS_val (1 << 0)
#define ADRJ_val (1 << 2)
#define GF2_val (1 << 3)
#define S2_P4_val (1 << 4)
#define SPI_P4_val (1 << 5)
#define PCA_P4_val (1 << 6)

#define NOP()						\
	__asm						\
		nop					\
	__endasm
		

#endif
