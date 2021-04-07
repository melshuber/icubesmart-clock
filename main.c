#include <stdint.h>
#include "framebuffer.h"
#include "uart.h"
#include "board.h"
#include "cpu.h"
#include "render.h"
#include "sim.h"
#include "fixed-point.h"
#include <math.h>

void test();

void render_frame(__xdata fb_frame_t *fb)
{
	static __xdata fp_t a = FP_sPI/16;//xbFP_0;
	static __xdata mat4x4_t m;

	fp_identity_mat4x4(&m);
	fp_pre_rotate_mat4x4_z(&m, a);

	render_clear(fb);
	render_tex2D(fb, &tex2, &m);

	a = fp_add(a, FP_sPI/16);
}

void bench_render_tex2d(__xdata fb_frame_t *fb)
{
	int8_t i;
	static __xdata mat4x4_t m;
	fp_identity_mat4x4(&m);

	for (i = 0; i < 100; i++) {
		render_tex2D(fb, &tex2, &m);
		sim_put_uint8(i);
	}
	sim_stop();
}

void main(void)
{
	while (!sim_detect());
	sim_puts("SIMULATION\n");

	//bench_render_tex2d(fb_back_frame());

	EA = 1;
	uart_init();
	fb_init();

	uart_puts("UART active\n");
	int i=0;

	while (1) {
		if (!fb_back_frame_complete()) {
			render_frame(fb_back_frame());
			render_sim_printfb(fb_back_frame());
			fb_back_frame_completed();
			i++;
			sim_put_uint8(i);
		}
	}
}
