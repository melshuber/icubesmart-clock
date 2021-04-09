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
__sbit __at(0x9A) RB8;
__sbit __at(0x9B) TB8;
__sbit __at(0x9C) REN;
__sbit __at(0x9D) SM2;
__sbit __at(0x9E) SM1;
__sbit __at(0x9F) SM0;
__sfr __at(0x99) SBUF;
__sfr __at(0x9C) BRT;

/* PCA Registers */
__sfr __at(0xD8) CCON;
__sbit __at(0xD8) CCF0;
__sbit __at(0xD9) CCF1;
__sbit __at(0xDE) CR;
__sbit __at(0xDF) CF;
__sfr __at(0xD9) CMOD;
__sfr __at(0xDA) CCAPM0;
__sfr __at(0xDB) CCAPM1;
__sfr __at(0xE9) CL;
__sfr __at(0xF9) CH;
__sfr __at(0xEA) CCAP0L;
__sfr __at(0xFA) CCAP0H;
__sfr __at(0xEB) CCAP1L;
__sfr __at(0xFB) CCAP1H;
__sfr __at(0xF2) PCA_PWM0;
__sfr __at(0xF3) PCA_PWM1;

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
__sfr __at(0xC7) IAP_CONTR;

/* Other Registers */
__sfr __at(0x8E) AUXR;
__sfr __at(0xA2) AUXR1;
