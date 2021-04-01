#include "framebuffer.h"
#include "cpu.h"
#include "board.h"

#if NOSIM_FB && SIMULATION
#undef SIMULATION
#define SIMULATION 0
#endif

#include "sim.h"

#include <string.h>

/* PLANE_CLKS
 *
 * Computed manually to avoid integer overflow
 *
 * DISPLAY_HZ = 20
 * PLANE_HZ = 8 * DISPLAY_Hz
 * FB_TIMER_HZ =  1_000_000  Hz
 *
 * PLANE_CLKS = FB_TIMER_CLK_HZ / PLANE_HZ
 *	      = 1_000_000 / (8 * 20)
 *	      = 6_250
 */
#define PLANE_CLKS 6250
#define FB_TIMER_TL_VAL ((65536 - PLANE_CLKS) & 0xff)
#define FB_TIMER_TH_VAL (((65536 - PLANE_CLKS) >> 8) & 0xff)

volatile uint8_t _fb_front_frame_idx = 0;
volatile uint8_t _fb_back_frame_complete = 0;
__xdata fb_frame_t _fb_frame[2];

static inline void _fb_enable_plane(uint8_t plane)
{
	PLANE_ENABLE = ~(1 << plane);
}

static inline void _fb_disable_planes(void)
{
	PLANE_ENABLE = 0xff;
}

void fb_init(void)
{
	sim_puts("fb_init\n");

	_fb_disable_planes();

	memset(&_fb_frame, 0, sizeof(_fb_frame));

	// Setup Timer as 16 Timer
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
	static uint8_t current_plane = 8;
	__xdata uint8_t *current_pixels = _fb_frame[0].pixels;

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

	/* before displaying the frame check if we need to flip
	 * it. */
	if ((current_plane == 0) && (fb_back_frame_complete())) {
		sim_puts("fb_flip\n");
		// flip frame
		_fb_front_frame_idx ^= 1;
		// indicate the back frame can be used
		_fb_back_frame_complete = 0;
	}

	if (current_plane >= 8) {
		current_pixels = fb_front_frame()->pixels;
		current_plane = 0;
	}

	_fb_disable_planes();

	// iterate over all bits in the LATCH_LOAD register
	uint8_t latch_load;
	for (latch_load = 1; latch_load != 0; latch_load <<= 1) {
		LATCH_DATA = *current_pixels;
		LATCH_LOAD = latch_load;
		NOP(); NOP();
		LATCH_LOAD = 0;
		current_pixels++;
	}

	_fb_enable_plane(current_plane);
	current_plane++;
}
