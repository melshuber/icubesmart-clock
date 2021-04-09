#include "board.h"
#include "cpu.h"
#include "uart.h"
#include "time.h"
#include "util.h"
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
static char _uart_data[32];
static uint8_t _uart_data_pos;

/* Command interface
 *
 * RX Line: "S:YYYYMMDDHHMMSS\n"
 *   Set Time
 * RX Line: "R\n"
 *   Reboot to ISP
 */

void uart_init(void) __critical
{
	uint8_t tmp;

	memset(_uart_data, 0, sizeof(_uart_data));
        _uart_data_pos = 0;
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

static void _uart_dispatch_command() __critical
{
	uint32_t i;
	if (strcmp(_uart_data, "R") == 0) {
		for (i = 1000000; i > 0; i--) {
			NOP();
			NOP();
			NOP();
			NOP();
			NOP();
			NOP();
			NOP();
			NOP();
		}
		IAP_CONTR = 0x60;  //0110_0000 soft reset system to run ISP
		return;
	}
	if (strncmp(_uart_data, "S:", 2) == 0) {
		time_t time;
		time.year_bcd = util_dec_to_bcd16(_uart_data + 2);
		time.month_bcd = util_dec_to_bcd8(_uart_data + 6);
		time.day_bcd = util_dec_to_bcd8(_uart_data + 8);
		time.hour_bcd = util_dec_to_bcd8(_uart_data + 10);
		time.min_bcd = util_dec_to_bcd8(_uart_data + 12);
		time.sec_bcd = util_dec_to_bcd8(_uart_data + 14);
		time_set(&time);
	}
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
		UART_RI = 0;

                if (tmp == '\r' || tmp == '\n') {
			_uart_dispatch_command();
			memset(_uart_data, 0, sizeof(_uart_data));
			_uart_data_pos = 0;
		} else if (_uart_data_pos < 31) {
			_uart_data[_uart_data_pos] = tmp;
			_uart_data_pos++;
		}
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
