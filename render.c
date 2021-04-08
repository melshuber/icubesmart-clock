#ifdef DEBUG_RENDER
#define DEBUG_MODULE render
#endif
#include "debug.h"

#include "render.h"
#include "fixed-point.h"

#include <string.h>

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

const tex2D_t tex1 = {
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
};

const tex2D_t tex2 = {
	0x0,
	0x3f << 1,
	0x3f << 1,
	0x3f << 1,
	0x3f << 1,
	0x3f << 1,
	0x3f << 1,
	0x0,
};

const tex2D_t tex3 = {
	0x18,
	0x18,
	0x18,
	0xFF,
	0xFF,
	0x18,
	0x18,
	0x18,
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

	if (check_debug(DEBUG_DEBUG)) {
		printf("tex\n");
		fp_print_mat4x4(transform);
		printf("vectors\n");
		printf("ux: "); fp_print_vec3(&render_tex2D_xseg.ux);
		printf("uy: "); fp_print_vec3(&render_tex2D_xseg.uy);
		printf("uz: "); fp_print_vec3(&render_tex2D_xseg.uz);
		printf("pz: "); fp_print_vec3(&render_tex2D_xseg.pz);
		printf("\n");
	};
}

#if !USE_RENDER_TEX2D_ASM || !USE_FAST_ADD_VEC3_ASM || !defined(FP_BITS_16) || (FP_EXP_BITS != 8)

//#define VEC3_ADD(O, A) fp_add_vec3_vec3((O), (O), (A))
#define VEC3_ADD(O, A) fp_fast_add_vec3((O), (A))

void render_tex2D(
	__xdata fb_frame_t *fb,
	const __xdata tex2D_t *tex,
	mat4x4_t *transform)
{
	int8_t x, y, z;

	render_tex2D_setup(transform);

	for (z = 0;
	     z < 8;
	     z++, VEC3_ADD(&render_tex2D_xseg.pz, &render_tex2D_xseg.uz))
	{
		memcpy(&render_tex2D_xseg.py, &render_tex2D_xseg.pz, sizeof(render_tex2D_xseg.py));
		for (y = 0;
		     y < 8;
		     y++, VEC3_ADD(&render_tex2D_xseg.py, &render_tex2D_xseg.uy))
		{
			memcpy(&render_tex2D_xseg.px, &render_tex2D_xseg.py, sizeof(render_tex2D_xseg.px));
			for (x = 0;
			     x < 8;
			     x++, VEC3_ADD(&render_tex2D_xseg.px, &render_tex2D_xseg.ux))
			{
				int8_t sx = FP_INT(render_tex2D_xseg.px[0]);
				int8_t sy = FP_INT(render_tex2D_xseg.px[1]);
				int8_t sz = FP_INT(render_tex2D_xseg.px[2]);
				/* printf("\n"); */
				/* printf("%d %d |", sx, sy); */
				/* fp_print_vec3(&render_tex2D_xseg.px); */

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

#else

/* Requires USE_FAST_ADD_VEC3_ASM, since the non-asm implementation
 * clobbers different registers. Otherwise it is an optimized variant
 * of the C implementation above. */

void render_tex2D(
	__xdata fb_frame_t *fb,
	const __xdata tex2D_t *tex,
	mat4x4_t *transform) __naked
{
	// hash the compiler
	(void)fb;
	(void)tex;
	(void)transform;
	__asm
		push	7 // r7 is used as z plane counter
		push	6 // r6 is used as y line counter
		push	5 // r5 is used as x pixel counter
		push	4 // r4 is used as tmp register to store pixels in line
		push	3 // r2 is used as high byte of the frame buffer address
		push	2 // r2 is used as low byte of the frame buffer address

		// push address of FP
		push	DPL
		push	DPH

		/* call render_tex2D_setup(transform) */
		mov	R0, #_render_tex2D_PARM_3
		movx	A, @R0
		mov	DPL, A
		inc	R0
		movx	A, @R0
		mov	DPH, A
		inc	R0
		movx	A, @R0
		mov	B, A
		lcall	_render_tex2D_setup

		/* Restore pushed FB onto R2,R3 */
		pop	3
		pop	2

		/* store &uz onto stack */
		mov	DPTR, #(_render_tex2D_xseg + 30)
		push	DPL
		push	DPH


		/*****************************/
		/* START OF LOOPs	     */

		/* for (z = 0; z != 8; z++) */
		mov	R7, #0
	0001$:
		/* store &uy onto stack */
		mov	DPTR, #(_render_tex2D_xseg + 18)
		push	DPL
		push	DPH

		/* Initialize py (start of current plane; this is pz)
		 *
		 * Note: &pz is currently the top of the stack */
		mov	DPTR, #(_render_tex2D_xseg + 12)
		inc	_AUXR1
		mov	DPTR, #(_render_tex2D_xseg + 24)
		lcall	0100$

		/* for (y = 0; y != 8; y++) */
		mov	R6, #0
	0002$:

		/* store &ux onto stack */
		mov	DPTR, #(_render_tex2D_xseg + 6)
		push	DPL
		push	DPH

		/* Initialize px (start of current line; this is py)
		 *
		 * Note: &py is currently the top of the stack */
		mov	DPTR, #(_render_tex2D_xseg + 0)
		inc	_AUXR1
		mov	DPTR, #(_render_tex2D_xseg + 12)
		lcall	0100$

		/* clear bits int row */
		mov	R4, #0
		/* for (x = 0; x != 8; x++) */
		mov	R5, #0
	0003$:
		/* tmp >>= 1 */
		mov	A, R4
		rr	A
		mov	R4, A

		/* check if FP_INT(px[2]) is within 0 to 7 */
		mov	DPTR, #(_render_tex2D_xseg + 5)
		movx	A, @DPTR
		jnz	0004$

		/* check if FP_INT(px[1]) is within 0 to 7 */
		mov	DPTR, #(_render_tex2D_xseg + 3)
		movx	A, @DPTR
		mov	R0, A	/* store offset within texture */
		anl	A, #0xF8
		jnz	0004$

		/* Load effective ROW of texture into R1 */
		mov	R1, #_render_tex2D_PARM_2
		movx	A, @R1
		add	A, R0
		mov	DPL, A
		inc	R1
		movx	A, @R1
		addc	A, #0
		mov	DPH, A
		movx	A, @DPTR
		mov	R1, A

		/* check if FP_INT(px[0]) is within 0 to 7 */
		mov	DPTR, #(_render_tex2D_xseg + 1)
		movx	A, @DPTR
		mov	R0, A
		anl	A, #0xF8
		jnz	0004$

		/* Move texture row into A, and FP_INT(px[0]) into
		 * R1, (exchange bytes) */
		inc	R0
		mov	A, R1
		/* now rotate the texture until the corresponding bit
		 * left most */
		sjmp	0006$
	0005$:
		rr	A
	0006$:
		djnz	R0, 0005$

		/* check if bit in texture is set */
		anl	A, #1
		jz	0004$

		/* set bit in row to render */
		mov	A, R4
		orl	A, #0x80
		mov	R4, A
	0004$:

		/* px += ux
		 *
		 * Note: &ux is the top of the stack */

		mov	DPTR, #(_render_tex2D_xseg)
		lcall	_fp_fast_add_vec3

		/* *****************************
		 * EOF LOOP on X
		 *
		 * repeat for while x >= 0 */
		inc	R5
		cjne	R5, #0x08, 0003$

		/* remove &py from stack */
		dec	SP
		dec	SP

		/* Store line into frame buffer */
		mov	DPL, R2
		mov	DPH, R3
		movx	A, @DPTR
		orl	A, R4
		movx	@DPTR, A
		inc	DPTR

		mov	R2, DPL
		mov	R3, DPH

		/* py += uy
		 *
		 * Note: &uy is the top of the stack */
		mov	DPTR, #(_render_tex2D_xseg + 12)
		lcall	_fp_fast_add_vec3

		/* *****************************
		 * EOF LOOP on Y
		 *
		 * repeat for while y >= 0 */
		inc	R6
		cjne	R6, #0x08, 0002$

		/* remove &py from stack */
		dec	SP
		dec	SP

		/* px += ux
		 *
		 * Note: &uz is the top of the stack */
		mov	DPTR, #(_render_tex2D_xseg + 24)
		lcall	_fp_fast_add_vec3

		/* *****************************
		 * EOF LOOP on Z
		 *
		 * repeat for while z >= 0 */
		inc	R7
		cjne	R7, #0x08, 0098$

		/* remove &pz from stack */
		dec	SP
		dec	SP

		pop	2
		pop	3
		pop	4
		pop	5
		pop	6
		pop	7

		ret

		/* Trampoline to outermost loop */
	0098$:
		ljmp	0001$
		/* mov	   R0, #_render_tex2D_z */
		/* mov	   A, #7 */
		/* movx	   @R0, A */

		/* SUBROUTINE 0099
		 *
		 * Copy six byte from active DPTR to other DPTR and
		 * switch the active DPTR */
	0100$:
	/* Byte 0 */
		movx   A, @DPTR
		inc    DPTR
		inc    _AUXR1
		movx   @DPTR, A
		inc    DPTR

	/* Byte 1 */
		inc    _AUXR1
		movx   A, @DPTR
		inc    DPTR
		inc    _AUXR1
		movx   @DPTR, A
		inc    DPTR

	/* Byte 2 */
		inc    _AUXR1
		movx   A, @DPTR
		inc    DPTR
		inc    _AUXR1
		movx   @DPTR, A
		inc    DPTR

	/* Byte 3 */
		inc    _AUXR1
		movx   A, @DPTR
		inc    DPTR
		inc    _AUXR1
		movx   @DPTR, A
		inc    DPTR

	/* Byte 4 */
		inc    _AUXR1
		movx   A, @DPTR
		inc    DPTR
		inc    _AUXR1
		movx   @DPTR, A
		inc    DPTR

	/* Byte 5 */
		inc    _AUXR1
		movx   A, @DPTR
		inc    DPTR
		inc    _AUXR1
		movx   @DPTR, A

		ret
	0099$:
	__endasm;
}
#endif

#define PLANES_PER_ROW 4

#pragma save
#pragma nogcse

void render_printfb(__xdata fb_frame_t *fb)
{
	static uint8_t x,y,z,j;

	printf("FB: %p\n", fb);

	for (z = 0; z < 8; z+=PLANES_PER_ROW) {
		for (j = 0; j < PLANES_PER_ROW; j++) {
			printf("Plane %d                 |  ", z+j);
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
