#ifdef DEBUG_FRAMEBUFFER
#define DEBUG_MODULE framebuffer
#endif

#include "debug.h"

#include "framebuffer.h"
#include "cpu.h"
#include "board.h"

#include <stdio.h>
#include <string.h>

/* PLANE_CLKS
 *
 * Computed manually to avoid integer overflow
 *
 * DISPLAY_HZ = 100
 * PLANE_HZ = 8 * DISPLAY_Hz
 * FB_TIMER_HZ =  2_000_000  Hz
 *
 * PLANE_CLKS = FB_TIMER_CLK_HZ / PLANE_HZ
 *	      = 2_000_000 / (8 * 100)
 *	      = 2_500
 */
#define PLANE_CLKS 2500
#define FB_TIMER_TL_VAL ((65536 - PLANE_CLKS) & 0xff)
#define FB_TIMER_TH_VAL (((65536 - PLANE_CLKS) >> 8) & 0xff)

volatile uint8_t _fb_front_frame_idx = 0;
volatile uint8_t _fb_back_frame_complete = 0;
__xdata fb_frame_t _fb_frame[2];
static __xdata uint8_t *_fb_current_pixels = _fb_frame[0].pixels;
static uint8_t _fb_current_plane = 0;

void fb_init(void) __critical
{
	/* disable planes */;
	PLANE_ENABLE = 0xff;

	memset(&_fb_frame, 0x55, sizeof(_fb_frame));
	_fb_front_frame_idx = 0;
	_fb_back_frame_complete = 0;
	_fb_current_pixels = fb_front_frame()->pixels;
	_fb_current_plane = 0;

	/* Setup Timer as 16 Timer */
	uint8_t tmp = FB_TIMER_MOD;
	tmp &= ~(0x0f << FB_TIMER_MOD_SHIFT);
	tmp |= (0x01 << FB_TIMER_MOD_SHIFT);
	FB_TIMER_MOD = tmp;

	FB_TIMER_TL = FB_TIMER_TL_VAL;
	FB_TIMER_TH = FB_TIMER_TH_VAL;

	FB_TIMER_IE = 1;
	FB_TIMER_RUN = 1;
}

void fb_timer_isr(void) __interrupt(FB_TIMER_IRQ) __using(1)
{
	/* Increase TL, TH to allow for best precision
	 * see STC89C51RC Manual sec. 7.2
	 */
	__asm
		CLR EA
		MOV A, FB_TIMER_TL
		ADD A, #FB_TIMER_TL_VAL
		MOV FB_TIMER_TL, A
		MOV A, FB_TIMER_TH
		ADDC A, #FB_TIMER_TH_VAL
		MOV FB_TIMER_TH, A
		SETB EA
	__endasm;

	/* disable planes */;
	PLANE_ENABLE = 0xff;

	// iterate over all bits in the LATCH_LOAD register
	uint8_t latch_load;
	for (latch_load = 1; latch_load != 0; latch_load <<= 1) {
		LATCH_DATA = ~*_fb_current_pixels;
		LATCH_LOAD = latch_load;
		LATCH_LOAD = 0;
		_fb_current_pixels++;
	}

	/* enable plane */;
	PLANE_ENABLE = ~(1 << _fb_current_plane);

	_fb_current_plane++;

	if (_fb_current_plane >= 8) {
		/* Check if we need to flip framebuffers */
		if (fb_back_frame_complete()) {
			// flip frame
			_fb_front_frame_idx ^= 1;
			// indicate the back frame can be used
			_fb_back_frame_complete = 0;
		}
		_fb_current_pixels = fb_front_frame()->pixels;
		_fb_current_plane = 0;
	}
}
