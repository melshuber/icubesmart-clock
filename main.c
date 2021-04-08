#ifdef DEBUG_MAIN
#define DEBUG_MODULE main
#endif
#include "debug.h"

#include <stdint.h>
#include "framebuffer.h"
#include "uart.h"
#include "board.h"
#include "cpu.h"
#include "render.h"
#include "sim.h"
#include "fixed-point.h"
#include "time.h"

static __xdata tex2D_t texture;

void render_frame(__xdata fb_frame_t *fb)
{
	static __xdata fp_t a = FP_0;
	static __xdata mat4x4_t m;

	fp_identity_mat4x4(&m);
	fp_pre_rotate_mat4x4_z(&m, a);

	render_clear(fb);
	render_tex2D(fb, &texture, &m);

	a = fp_add(a, FP_sPI/16);
}

void bench_render_tex2d(__xdata fb_frame_t *fb)
{
	int8_t i;
	static __xdata mat4x4_t m;
	fp_identity_mat4x4(&m);

	for (i = 0; i < 100; i++) {
		render_tex2D(fb, &texture, &m);
		note("%02x", i);
	}
	sim_stop();
}

void pca_init() __critical
{
	uint8_t tmp;

	tmp = AUXR1;
	tmp &= ~PCA_P4_val;
	AUXR1 = tmp;

	CL = 0;
	CH = 0;
	CMOD = CPS_div12;
	CCON = CR_val;
}

void pca_isr() __interrupt(7)
{
	if (TIME_CCF) {
		TIME_CCF = 0;
		time_isr();
	}
}

void main(void)
{
	memcpy(&texture, &tex1, sizeof(tex2D_t));

	//bench_render_tex2d(fb_back_frame());

	EA = 1;
	uart_init();
	printf("iCube started [%s]\n",
	       sim_detect() ? "SIM" : "LIVE");

	fb_init();
	printf("Framebuffer initialized\n");

	time_init();
	printf("Time initialized\n");

	pca_init();
	printf("PCA initialized\n");

	uint8_t i=0;

	while (1) {
		if (!fb_back_frame_complete()) {
			render_frame(fb_back_frame());
			if (check_debug(DEBUG_DEBUG))
			    render_printfb(fb_back_frame());
			fb_back_frame_completed();
			note("Frame: %d\n", i);
			i++;
		}
	}
}
