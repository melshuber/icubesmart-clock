/* GPIOs to 3d Matrix Display System */
__sfr __at(0x80) P0 = 0;
__sfr __at(0x90) P1 = 0;
__sfr __at(0xA0) P2 = 0;

/* Timer 0/1 Registers */
__sfr __at(0x88) TCON = 0;
__sbit __at(0x88) IT0;
__sbit __at(0x89) IE0;
__sbit __at(0x8A) IT1;
__sbit __at(0x8B) IE1;
__sbit __at(0x8C) TR0;
__sbit __at(0x8D) TF0;
__sbit __at(0x8E) TR1;
__sbit __at(0x8F) TF1;
__sfr __at(0x89) TMOD = 0;
__sfr __at(0x8A) TL0 = 0;
__sfr __at(0x8B) TL1 = 0;
__sfr __at(0x8C) TH0 = 0;
__sfr __at(0x8D) TH1 = 0;

/* Timer 2 Registers */
__sfr __at(0xC8) T2CON = 0;
__sfr __at(0xC9) T2MOD = 0;
__sfr __at(0xCA) RCAP2L = 0;
__sfr __at(0xCB) RCAP2H = 0;
__sfr __at(0xCC) TL2 = 0;
__sfr __at(0xCD) TH2 = 0;

/* UART Registers */

__sfr __at(0x98) SCON = 0;
__sbit __at(0x98) RI;
__sbit __at(0x99) TI;
__sbit __at(0x9C) REN;
__sfr __at(0x99) SBUF;

/* Interrupt Registers */
__sfr __at(0xA8) IE = 0;
__sbit __at (0xA8) EX0;
__sbit __at (0xA9) ET0;
__sbit __at (0xAA) EX1;
__sbit __at (0xAB) ET1;
__sbit __at (0xAC) ES;
__sbit __at (0xAF) EA;

/* POWER Registers */
__sfr __at(0x87) PCON;

/* ISP Registers */
__sfr __at(0xE7) ISP_CONTR;
