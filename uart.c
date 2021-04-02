#include "board.h"
#include "uart.h"
#include <stdint.h>

#if NOSIM_UART && SIMULATION
#undef SIMULATION
#define SIMULATION 0
#endif

#include "sim.h"

#define BAUD 9600

/* BAUD_CLKS
 *
 * Computed manually to avoid integer overflow
 *
 * BAUD = 9600
 * UART_DIVIDER = 16
 * UART_TIMER_HZ =  1_000_000Hz
 *
 * PLANE_CLKS = UART_TIMER_CLK_HZ / (BAUD * UART_DIVIDER)
 *            = 1_000_000 / (9600 * 16)
 *            = 6.51
 */
#define BAUD_CLKS 7

static volatile __bit _uart_busy = 0;

void uart_init(void)
{
	sim_puts("uart_init\n");

        uint8_t tmp;
	// Setup Timer as 8 Bit Timer with auto reloade
	tmp = UART_TIMER_MOD;
	tmp &= ~(0x0f << UART_TIMER_MOD_SHIFT);
	tmp |= (0x02 << UART_TIMER_MOD_SHIFT);
	UART_TIMER_MOD = tmp;

	UART_TIMER_TL = (uint8_t)(256 - BAUD_CLKS);
	UART_TIMER_TH = (uint8_t)(256 - BAUD_CLKS);

	UART_TIMER_RUN = 1;

	// set SMOD in PCON register to double baudrate
	tmp = PCON;
	tmp |= (1 << 7);
	PCON = tmp;

	UART_REN = 1;
        UART_IE = 1;
}

void uart_isr() __interrupt(UART_IRQ)
{
	uint8_t tmp;
	if (UART_TI) {
		sim_puts("uart_tx_complete\n");
		UART_TI = 0;
		_uart_busy = 0;
	}

	if (UART_RI) {
		sim_puts("uart_rx_complete\n");
		tmp = SBUF;

		if (tmp == 'R') {
			sim_puts("uart_rx reboot to ISP\n");
			ISP_CONTR = 0x60;  //0110_0000 soft reset system to run ISP
		}

                UART_RI = 0;
	}
}

void uart_putc(char c)
{
	sim_puts("uart_tx\n");
	while (_uart_busy) ;
	_uart_busy = 1;
	SBUF = c;
}

void uart_puts(__code const char *str)
{
	while (*str) {
		uart_putc(*str);
		str++;
	}
}
