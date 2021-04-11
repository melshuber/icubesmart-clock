#ifndef BOARD_H
#define BOARD_H

#include "cpu.h"

#define M(X) ((X) * 1000000)
#define K(X) (Hz(X) * 1000)
#define Hz(X) (X)

#define CPU_CLK_HZ M(Hz(24))
#define TIMER_CLK_HZ (CPU_CLK_HZ / 12)

/* Key IO Configure */
#define KEY_START 0
#define KEY_CYCLE 1
#define KEY_NEXT 2
#define KEY_START_BIT P4_1
#define KEY_CYCLE_BIT P4_2
#define KEY_NEXT_BIT P4_3
#define KEY_PORT P4
#define KEY_POLL_HZ TIME_TICK_HZ
#define KEY_SHORT_TICKS (TIME_TICK_HZ / 10)
#define KEY_LONG_TICKS (TIME_TICK_HZ * 2)

/* Configure Frame Buffer */
#define FB_CLK_HZ PCA_HZ
#define FB_TICK_HZ Hz(800)
#define FB_TICKS (FB_CLK_HZ/FB_TICK_HZ)
#define FB_CCF CCF1
#define FB_CCAPM CCAPM1
#define FB_ECCF_val ECCF1_val
#define FB_PWM_val PWM1_val
#define FB_TOG_val TOG1_val
#define FB_MAT_val MAT1_val
#define FB_CAPN_val CAPN1_val
#define FB_CAPP0_val CAPP1_val
#define FB_ECOM_val ECOM1_val
#define FB_CCAPL CCAP1L
#define FB_CCAPH CCAP1H
#define FB_PCA_PWM PCA_PWM1

#define LATCH_DATA P0
#define LATCH_LOAD P2
#define PLANE_ENABLE P1

/* Configure UART */
#define UART_IRQ 4
#define UART_RI RI
#define UART_TI TI
#define UART_REN REN
#define UART_IE ES

/* Configure PCA */
#define PCA_HZ (CPU_CLK_HZ / 12)

/* Configure Time */
#define TIME_CLK_HZ PCA_HZ
#define TIME_TICK_HZ Hz(50)
#define TIME_TICKS (TIME_CLK_HZ/TIME_TICK_HZ)
#define TIME_CCF CCF0
#define TIME_CCAPM CCAPM0
#define TIME_ECCF_val ECCF0_val
#define TIME_PWM_val PWM0_val
#define TIME_TOG_val TOG0_val
#define TIME_MAT_val MAT0_val
#define TIME_CAPN_val CAPN0_val
#define TIME_CAPP0_val CAPP0_val
#define TIME_ECOM_val ECOM0_val
#define TIME_CCAPL CCAP0L
#define TIME_CCAPH CCAP0H
#define TIME_PCA_PWM PCA_PWM0

#endif
