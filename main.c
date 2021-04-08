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
#include "font.h"

static __xdata union {
	struct {
		mat4x4_t m;
		tex2D_t texture;
	} bench_render_tex2d;
	struct {
		mat4x4_t m;
		tex2D_t texture;
	} render_frame;
	struct {
		mat4x4_t m;
		vec3_t v;
		tex2D_t texture;
	} render_digits;
} _xso;

typedef struct {
	uint16_t old_digits;
	uint16_t new_digits;
	uint16_t ticks;
	uint8_t step;
	uint8_t pending;
} digits_animation_t;

static __xdata digits_animation_t _digits_animation;

#define XSO(X) _xso.OVERLAY_FIELD.X
#define xso_m XSO(m)
#define xso_texture XSO(texture)

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

#define OVERLAY_FIELD render_frame
void render_frame(__xdata fb_frame_t *fb)
{
	static __xdata fp_t a = FP_0;
	memcpy(&XSO(texture),
	       font_get_texture('4'),
	       sizeof(tex2D_t));

	fp_identity_mat4x4(&XSO(m));
	fp_pre_rotate_mat4x4_z(&XSO(m), a);

	render_clear(fb);
	render_tex2D(fb, &XSO(texture), &XSO(m));

	a = fp_add(a, FP_sPI/16);
}
#undef OVERLAY_FIELD

#define OVERLAY_FIELD render_frame
void bench_render_tex2d(__xdata fb_frame_t *fb)
{
	int8_t i;

	memset(&XSO(texture), 0xff, sizeof(tex2D_t));
        fp_identity_mat4x4(&XSO(m));

	for (i = 0; i < 100; i++) {
		render_tex2D(fb, &XSO(texture), &XSO(m));
		note("%02x", i);
	}
	sim_stop();
}
#undef OVERLAY_FIELD

#define OVERLAY_FIELD render_digits
void render_digits(__xdata fb_frame_t *fb) __reentrant
{
	int i;
	static const mat4x4_t mat_rotate = {
		{  FP_1,  FP_0,  FP_0,  FP_0, },
		{  FP_0,  FP_0, -FP_1,  FP_0, },
		{  FP_0,  FP_1,  FP_0,  FP_0, },
		{  FP_1,  FP_0,  FP_0,  FP_1, },
	};

	static const vec4_t pos[4] = {
		{ FP_1, fp_add(-FP_3, -FP_1/2), FP_0 },
		{ -FP_1, fp_add(-FP_1, -FP_1/2), FP_0 },
		{ FP_1, fp_add(FP_1, FP_1/2), FP_0 },
		{ -FP_1, fp_add(FP_3, FP_1/2), FP_0 },
	};

	uint16_t new_digits = _digits_animation.new_digits;
	uint16_t old_digits = _digits_animation.old_digits;	

	for (i = 0; i < 4; i++) {
		uint8_t new_digit = new_digits & 0xf;
		uint8_t old_digit = old_digits & 0xf;

		memcpy(&XSO(v), &pos[i], sizeof(vec3_t));
		if ((new_digit == old_digit) || (_digits_animation.step > 8)) {
			memcpy(&XSO(texture),
			       font_get_texture('0' + new_digit),
			       sizeof(tex2D_t));
		} else {
			XSO(v)[1] = fp_sub(XSO(v)[1], FP_FROM_INT(_digits_animation.step));
			if (XSO(v)[1] < -FP_4) {
				XSO(v)[1] = fp_add(XSO(v)[1], FP_FROM_INT(8));
				memcpy(&XSO(texture),
				       font_get_texture('0' + new_digit),
				       sizeof(tex2D_t));
			} else {
				memcpy(&XSO(texture),
				       font_get_texture('0' + old_digit),
				       sizeof(tex2D_t));
			}
		}

                memcpy(&XSO(m), &mat_rotate, sizeof(mat4x4_t));
		fp_post_translate_mat4x4_vec3(&XSO(m), &XSO(v));

                render_tex2D(fb, &XSO(texture), &XSO(m));

		new_digits >>= 4;
		old_digits >>= 4;
	}
	_digits_animation.pending = 0;
}
#undef OVERLAY_FIELD

void main(void)
{
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

	uint16_t tmp;
	time_t time;
	time_get(&time);

        uint8_t i=0;
	_digits_animation.old_digits = 0xffff;
	_digits_animation.new_digits = 0xffff;
	_digits_animation.step = 9;
	_digits_animation.ticks = time.ticks;
	_digits_animation.pending = 1;

	for (;;PCON |= 1) {
		if (fb_back_frame_complete()) {
			continue;
		}

                time_get(&time);

		__xdata fb_frame_t *fb = fb_back_frame();

                if (_digits_animation.step > 8) {
			tmp = ((uint16_t)time.min_bcd) << 8;
			tmp |= time.sec_bcd;

			if (_digits_animation.new_digits != tmp) {
				_digits_animation.old_digits = _digits_animation.new_digits;
				_digits_animation.new_digits = tmp;
				_digits_animation.ticks = time.ticks;
				_digits_animation.step = 0;
				_digits_animation.pending = 1;
			}
		} else {
			tmp = time.ticks - _digits_animation.ticks;
			uint8_t tmp2 = _digits_animation.step;
			if (tmp >= 36) {
				_digits_animation.step = 9;
			} else {
				_digits_animation.step = ((uint8_t)tmp) / 4;
			}
			if (tmp2 != _digits_animation.step) {
				_digits_animation.pending = 1;
			}
		}

		if (_digits_animation.pending != 0) {
			render_clear(fb);

			uint16_t ticks = time.ticks;
			render_digits(fb);

			time_get(&time);

                        fb_back_frame_completed();
			note("Frame: %d; ticks: %u, %d\n", i, time.ticks - ticks, ticks);
			i++;
		}
	}
}
