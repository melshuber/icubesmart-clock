#include "render.h"
#include "fixed-point.h"

#include <string.h>

#if NOSIM_RENDER && SIMULATION
#undef SIMULATION
#define SIMULATION 0
#endif

#include "sim.h"

static const vec4_t normal_x = {
	FP_1,
	FP_0,
	FP_0,
	FP_0,
};

static const vec4_t normal_y = {
	FP_0,
	FP_1,
	FP_0,
	FP_0,
};

static const vec4_t normal_z = {
	FP_0,
	FP_0,
	FP_1,
	FP_0,
};

/* we start at the top most positive corner render */
static const vec4_t origin = {
	FP_4 - FP_1/2,
	FP_4 - FP_1/2,
	FP_4 - FP_1/2,
	FP_1,
};

/* we start at the top most positive corner render */
static const vec4_t origin2 = {
	FP_1/2 - FP_4,
	FP_1/2 - FP_4,
	FP_1/2 - FP_4,
	FP_1,
};

const __xdata tex2D_t tex1 = {
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
};

const __xdata tex2D_t tex2 = {
	0x0,
	0x3f << 1,
	0x3f << 1,
	0x3f << 1,
	0x3f << 1,
	0x3f << 1,
	0x3f << 1,
	0x0,
};

void render_clear(__xdata fb_frame_t *fb)
{
	memset(fb, 0, sizeof(*fb));
}

void render_set_pixel(
	__xdata fb_frame_t *fb,
	uint8_t x,
	uint8_t y,
	uint8_t z)
{
	uint8_t idx = z * 8 + y;
	fb->pixels[idx] |= (1 << x);
}

static __xdata struct {
	vec3_t px, ux;
	vec3_t py, uy;
	vec3_t pz, uz;
} render_tex2D_xseg;

static void render_tex2D_setup(mat4x4_t *transform)
{
	static const vec3_t tex2D_shift = {
		FP_4,
		FP_4,
		FP_1/2,
	};

	fp_mul_mat4x4_vec4_3(&render_tex2D_xseg.ux, transform, &unit_x.v4);
	fp_mul_mat4x4_vec4_3(&render_tex2D_xseg.uy, transform, &unit_y.v4);
	fp_mul_mat4x4_vec4_3(&render_tex2D_xseg.uz, transform, &unit_z.v4);
	fp_mul_mat4x4_vec4_3(&render_tex2D_xseg.pz, transform, &origin2);
	fp_add_vec3_vec3(&render_tex2D_xseg.pz, &render_tex2D_xseg.pz, &tex2D_shift);

	/* printf("tex\n"); */
	/* fp_print_mat4x4(transform); */
	/* printf("vectors\n"); */
	/* printf("ux: "); fp_print_vec3(&render_tex2D_xseg.ux); */
	/* printf("uy: "); fp_print_vec3(&render_tex2D_xseg.uy); */
	/* printf("uz: "); fp_print_vec3(&render_tex2D_xseg.uz); */
	/* printf("pz: "); fp_print_vec3(&render_tex2D_xseg.pz); */
	/* printf("\n"); */
}

void render_tex2D(
	__xdata fb_frame_t *fb,
	const __xdata tex2D_t *tex,
	mat4x4_t *transform)
{
	int8_t x, y, z;

	render_tex2D_setup(transform);

	for (z = 0;
	     z < 8;
	     z++, fp_add_vec3_vec3(&render_tex2D_xseg.pz, &render_tex2D_xseg.pz, &render_tex2D_xseg.uz))
	{
		memcpy(&render_tex2D_xseg.py, &render_tex2D_xseg.pz, sizeof(render_tex2D_xseg.py));
		for (y = 0;
		     y < 8;
		     y++, fp_add_vec3_vec3(&render_tex2D_xseg.py, &render_tex2D_xseg.py, &render_tex2D_xseg.uy))
		{
			memcpy(&render_tex2D_xseg.px, &render_tex2D_xseg.py, sizeof(render_tex2D_xseg.px));
			for (x = 0;
			     x < 8;
			     x++, fp_add_vec3_vec3(&render_tex2D_xseg.px, &render_tex2D_xseg.px, &render_tex2D_xseg.ux))
			{
				int8_t sx = FP_INT(render_tex2D_xseg.px[0]);
				int8_t sy = FP_INT(render_tex2D_xseg.px[1]);
				int8_t sz = FP_INT(render_tex2D_xseg.px[2]);
				/* printf("\n"); */
				/* printf("%d %d |", sx, sy); */
				/* fp_print_vec3(&px); */
//
				if ((sz != 0) ||
				    (sy < 0) ||
				    (sy >= 8) ||
				    (sx < 0) ||
				    (sx >= 8))
					continue;
				if ((*tex)[sy] & (1 << sx)) {
					render_set_pixel(fb, x, y, z);
				}
			}
		}
	}
}

#if SIMULATION
#define PLANES_PER_ROW 8

#pragma save
#pragma nogcse

void render_sim_printfb(__xdata fb_frame_t *fb)
{
	static uint8_t x,y,z,j;

	sim_puts("FB: ");
	sim_put_intptr((intptr_t)fb);
	sim_putc('\n');

	for (z = 0; z < 8; z+=PLANES_PER_ROW) {
		for (j = 0; j < PLANES_PER_ROW; j++) {
			printf("Plane %d		 |  ", z+j);
		}
		putchar('\n');
		for (y = 0; y < 8; y++) {
			for (j = 0; j < PLANES_PER_ROW; j++) {
				printf("%02x %02x: ",
				       (z + j) * 8 + y,
				       fb->pixels[(z + j) * 8 + y]);
				for (x = 0; x < 8; x++) {
					if (fb->pixels[(z  + j) * 8 + y] & (1 << x)) {
						putchar('#');
					} else {
						putchar('*');
					}
					putchar(' ');
				}
				printf(" |  ");
			}
			putchar('\n');
		}
	}
}
#pragma restore

#endif
