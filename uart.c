#include "board.h"
#include "cpu.h"
#include "uart.h"
#include <stdint.h>

#if NOSIM_UART && SIMULATION
#undef SIMULATION
#define SIMULATION 0
#endif

#include "sim.h"

#define BAUD 115200
#define BAUD_CLKS (CPU_CLK_HZ / 16 / BAUD)

static volatile __bit _uart_busy;
static volatile __bit _uart_redirect_to_simulation;
static volatile __bit _uart_need_nl;

void uart_init(void) __critical
{
	uint8_t tmp;

	_uart_busy = 0;
        _uart_need_nl = 0;
        _uart_redirect_to_simulation = 0;

	if (sim_detect()) {
		_uart_redirect_to_simulation = 1;
	}

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
		UART_TI = 0;

		if (_uart_need_nl) {
			SBUF = '\n';
			_uart_need_nl = 0;
		} else {
			_uart_busy = 0;
		}
	}

	if (UART_RI) {
		tmp = SBUF;

		if (tmp == 'R') {
			IAP_CONTR = 0x60;  //0110_0000 soft reset system to run ISP
		}

		UART_RI = 0;
	}
}

void uart_putc(char c)
{
	while (_uart_busy) ;
	_uart_busy = 1;

	// insert a \r before \n
        if (c == '\n') {
		_uart_need_nl = 1;
                SBUF = '\r';
		return;
	}

	SBUF = c;
}

void uart_puts(__code const char *str)
{
	while (*str) {
		uart_putc(*str);
		str++;
	}
}

int putchar(int c)
{
	if (EA) {
		if (_uart_redirect_to_simulation) {
			sim_putc(c);
		} else {
			uart_putc(c);
		}
	}
	return c;
}
