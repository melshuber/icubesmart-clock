#include <stdint.h>
#include "framebuffer.h"
#include "board.h"
#include "cpu.h"
#include "sim.h"

#define T2CON_TR2 (1 << 2)

void render_frame(__xdata fb_frame_t *frame)
{
	uint8_t i;

	for (i = 0; i < sizeof(frame); i++) {
		frame->pixels[i] = i;
	}
}

void main(void)
{
	while (!sim_detect());
	sim_puts("SIMULATION\n");

	EA = 1;
	fb_init();

	/* sim_stop(); */
	/* sim_stop(); */
	/* sim_stop(); */
	/* sim_stop(); */

	while (1) {
		if (!fb_back_frame_complete()) {
			render_frame(fb_back_frame());
			fb_back_frame_completed();			
		}
	}
}
