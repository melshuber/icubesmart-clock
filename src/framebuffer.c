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
__xdata fb_frame_t FB_FRAME_BUFFER_ATTR _fb_frame[2] = {
	{
		.pixels = { 0 },
	},
	{
		.pixels = { 0 },
	},
};
static __xdata uint8_t *_fb_current_pixels = _fb_frame[0].pixels;
static uint8_t _fb_current_plane = 0;
static uint16_t _fb_ccap_val = 0;

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
	_fb_ccap_val = FB_TICKS;
	FB_CCAPL = FB_TICKS & 0xff;
	FB_CCAPH = (FB_TICKS >> 8) & 0xff;
	FB_PCA_PWM = 0;
	FB_CCAPM = FB_ECOM_val | FB_MAT_val | FB_ECCF_val;
}

void fb_isr(void)
{
	uint16_t tmp = _fb_ccap_val;
	tmp += FB_TICKS;
	_fb_ccap_val = tmp;
	FB_CCAPL = tmp & 0xff;
	FB_CCAPH = (tmp >> 8) & 0xff;

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
