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
#include "key.h"

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
	} mode_clock_render;
} _xso;

typedef enum {
	MODE_CLOCK,
} mode_t;

typedef struct {
	mode_t mode;
	time_t time;
	union {
		struct {
			uint16_t start_tick;
			uint8_t step;
			uint16_t old_digits;
			uint16_t new_digits;
			uint8_t pending;
		} mode_clock;
	};
} display_state_t;

static __xdata display_state_t _display_state;

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
	if (FB_CCF) {
		FB_CCF = 0;
		fb_isr();
	}
	if (TIME_CCF) {
		TIME_CCF = 0;
		time_isr();
		key_isr();
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

static const mat4x4_t mat_rotate = {
	{  FP_0,  FP_1,	 FP_0,	FP_0, },
	{  FP_0,  FP_0, -FP_1,	FP_0, },
	{ -FP_1,  FP_0,	 FP_0,	FP_0, },
	{  FP_0,  FP_0,	 FP_0,	FP_1, },
};

static const vec4_t pos[4] = {
	{ fp_add(FP_3, FP_1/2), FP_1, FP_0 },
	{ fp_add(FP_1, FP_1/2), -FP_1, FP_0 },
	{ fp_add(-FP_1, -FP_1/2), FP_1, FP_0 },
	{ fp_add(-FP_3, -FP_1/2), -FP_1, FP_0 },
};

#define OVERLAY_FIELD mode_clock_render
static void _mode_clock_render(__xdata fb_frame_t *fb) __reentrant
{
	int i;

	uint16_t new_digits = _display_state.mode_clock.new_digits;
	uint16_t old_digits = _display_state.mode_clock.old_digits;

	for (i = 0; i < 4; i++) {
		uint8_t new_digit = new_digits & 0xf;
		uint8_t old_digit = old_digits & 0xf;

		memcpy(&XSO(v), &pos[i], sizeof(vec3_t));
		if ((new_digit == old_digit) || (_display_state.mode_clock.step > 8)) {
			memcpy(&XSO(texture),
			       font_get_texture('0' + new_digit),
			       sizeof(tex2D_t));
		} else {
			if (XSO(v)[0] > 0) {
				XSO(v)[0] = fp_add(XSO(v)[0], FP_FROM_INT(_display_state.mode_clock.step));
				if (XSO(v)[0] > FP_4) {
					XSO(v)[0] = fp_sub(XSO(v)[0], FP_FROM_INT(8));
					memcpy(&XSO(texture),
					       font_get_texture('0' + new_digit),
					       sizeof(tex2D_t));
				} else {
					memcpy(&XSO(texture),
					       font_get_texture('0' + old_digit),
					       sizeof(tex2D_t));
				}
			} else {
				XSO(v)[0] = fp_sub(XSO(v)[0], FP_FROM_INT(_display_state.mode_clock.step));
				if (XSO(v)[0] < -FP_4) {
					XSO(v)[0] = fp_add(XSO(v)[0], FP_FROM_INT(8));
					memcpy(&XSO(texture),
					       font_get_texture('0' + new_digit),
					       sizeof(tex2D_t));
				} else {
					memcpy(&XSO(texture),
					       font_get_texture('0' + old_digit),
					       sizeof(tex2D_t));
				}
			}
		}

		memcpy(&XSO(m), &mat_rotate, sizeof(mat4x4_t));

		fp_post_translate_mat4x4_vec3(&XSO(m), &XSO(v));

		render_tex2D(fb, &XSO(texture), &XSO(m));

		new_digits >>= 4;
		old_digits >>= 4;
	}
	_display_state.mode_clock.pending = 0;
}
#undef OVERLAY_FIELD

static uint16_t _mode_clock_digits()
{
	uint16_t tmp;
	tmp = ((uint16_t)_display_state.time.hour_bcd) << 8;
	tmp |= _display_state.time.min_bcd;
	return tmp;
}

static void _mode_clock_setup()
{
	uint16_t tmp = _mode_clock_digits();
	_display_state.mode = MODE_CLOCK;
	_display_state.mode_clock.old_digits = tmp;
	_display_state.mode_clock.new_digits = tmp;
	_display_state.mode_clock.step = 9;
	_display_state.mode_clock.start_tick = 0;
	_display_state.mode_clock.pending = 1;
}

static void _mode_clock_handle()
{
	uint16_t tmp;

	/* Check if a back framebuffer is available */
	if (fb_back_frame_complete()) {
		/* early exit if not */
		return;
	}

	/* update animation */
	if (_display_state.mode_clock.step > 8) {
		tmp = _mode_clock_digits();

		if (_display_state.mode_clock.new_digits != tmp) {
			_display_state.mode_clock.old_digits =
				_display_state.mode_clock.new_digits;
			_display_state.mode_clock.new_digits = tmp;
			_display_state.mode_clock.start_tick = _display_state.time.ticks;
			_display_state.mode_clock.step = 0;
			_display_state.mode_clock.pending = 1;
		}
	} else {
		tmp = _display_state.time.ticks - _display_state.mode_clock.start_tick;
		uint8_t tmp2 = _display_state.mode_clock.step;
		if (tmp >= 36) {
			_display_state.mode_clock.step = 9;
		} else {
			_display_state.mode_clock.step = ((uint8_t)tmp) / 4;
		}
		if (tmp2 != _display_state.mode_clock.step) {
			_display_state.mode_clock.pending = 1;
		}
	}

	/* render frame */
	if (_display_state.mode_clock.pending != 0) {
		__xdata fb_frame_t *fb = fb_back_frame();
		render_clear(fb);
		_mode_clock_render(fb);
		fb_back_frame_completed();
		note("f\n");
	}
}

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

	key_init();
	printf("Keys initialized\n");

	pca_init();
	printf("PCA initialized\n");

	time_get(&_display_state.time);

	uint8_t i=0;
	_mode_clock_setup();

	for (;;PCON |= 1) {
		time_get(&_display_state.time);
		switch (_display_state.mode) {
		default:
			_mode_clock_setup();
		case MODE_CLOCK:
			_mode_clock_handle();
			break;
		}
	}
}
