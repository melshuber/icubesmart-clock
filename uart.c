#include "board.h"
#include "cpu.h"
#include "uart.h"
#include <stdint.h>

#if !SIMULATION
int putchar(int __c)
{
	if (EA) {
		uart_putc(__c);
	}
	return __c;
}
#endif

#if NOSIM_UART && SIMULATION
#undef SIMULATION
#define SIMULATION 0
#endif

#include "sim.h"

#define BAUD 115200
#define BAUD_CLKS (CPU_CLK_HZ / 16 / BAUD)

static volatile __bit _uart_busy = 0;
void uart_init(void)
{
	uint8_t tmp;
	sim_puts("uart_init\n");

	/* Setup BRT - BAUD 115200 */
	BRT = (256 - BAUD_CLKS) & 0xff;

	/* Setup UART1 into Mode 1 */
	SM0 = 0;
	SM1 = 1;
	SM2 = 0;

	tmp = PCON;
	tmp &= ~(PCON_SMOD0_val | PCON_SMOD_val);
	tmp |= PCON_SMOD_val;
	PCON = tmp;

	/* Select and enable dedicated bautrate generator */
	tmp = AUXR;
	tmp &= ~(S1BRS_val | BRTR_val | BRTx12_val);
	tmp |= S1BRS_val | BRTR_val | BRTx12_val;
	AUXR = tmp;

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
			SBUF = 'A';
			sim_puts("uart_rx reboot to ISP\n");
			IAP_CONTR = 0x60;  //0110_0000 soft reset system to run ISP
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
