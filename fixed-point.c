#include "fixed-point.h"
#include <math.h>
#include <stdint.h>
#include <string.h>

#include "sim.h"

const u_vec4_t unit_x = {
	.v4 = {
		FP_1,
		FP_0,
		FP_0,
		FP_0,
	}
};

const u_vec4_t unit_y = {
	.v4 = {
		FP_0,
		FP_1,
		FP_0,
		FP_0,
	}
};

const u_vec4_t unit_z = {
	.v4 = {
		FP_0,
		FP_0,
		FP_1,
		FP_0,
	}
};

const u_vec4_t unit_mx = {
	.v4 = {
		-FP_1,
		FP_0,
		FP_0,
		FP_0,
	}
};

const u_vec4_t unit_my = {
	.v4 = {
		FP_0,
		-FP_1,
		FP_0,
		FP_0,
	}
};

const u_vec4_t unit_mz = {
	.v4 = {
		FP_0,
		FP_0,
		-FP_1,
		FP_0,
	}
};

/* XSEG manual overlay data */
static __xdata union {
	struct {
		vec4_t r2;
		vec4_t r3;
	} fp_pre_rotate_mat4x4_x;
	struct {
		vec4_t r1;
		vec4_t r3;
	} fp_pre_rotate_mat4x4_y;
	struct {
		vec4_t r1;
		vec4_t r2;
	} fp_pre_rotate_mat4x4_z;
} _xso;
#define XSO(X) _xso.OVERLAY_FIELD.X
#define xso_r1 XSO(r1)
#define xso_r2 XSO(r2)
#define xso_r3 XSO(r3)

/* PSEG manual overlay data */
static __pdata union {
	struct {
		fp_t tmp;
		int8_t i, row, col;
	} fp_mul_mat4x4_mat4x4;
	struct {
		fp_t tmp;
		int8_t i, row;
	} fp_mul_mat4x4_vec4_3;
	struct {
		fp_t tmp;
		int8_t i, row;
	} fp_mul_mat4x4_vec4_4;
	struct {
		fp_t tmp;
		int8_t row, col;
	} fp_pre_translate_mat4x4_vec3;
	struct {
		fp_t tmp;
		int8_t row, col;
	} fp_post_translate_mat4x4_vec3;
	struct {
		fp_t s, c;
		int8_t i;
	} fp_pre_rotate_mat4x4_x, fp_pre_rotate_mat4x4_y, fp_pre_rotate_mat4x4_z;
#if WITH_FP_PRE_ROTATION
	struct {
		fp_t s, c, inv_c;
	} fp_pre_rotation_mat4x4;
#endif
} _pso;

#define PSO(X) _pso.OVERLAY_FIELD.X
#define pso_tmp PSO(tmp)
#define pso_row PSO(row)
#define pso_col PSO(col)
#define pso_i PSO(i)
#define pso_s PSO(s)
#define pso_c PSO(c)
#define pso_inv_c PSO(inv_c)

#if SIMULATION
#pragma save
#pragma nogcse
void fp_print_vec4(const vec4_t *v)
{
	uint8_t row;
	for (row = 0; row < 4; row++) {
		printf("%8d/%d",
		       (*v)[row],
		       FP_EXP_POW2);
	}
	printf("\n");
}

void fp_print_vec3(const vec3_t *v)
{
	uint8_t row;
	for (row = 0; row < 3; row++) {
		printf("%8d/%d",
		       (*v)[row],
		       FP_EXP_POW2);
	}
	printf("\n");
}

void fp_print_mat4x4(const mat4x4_t *m)
{
	uint8_t row, col;
	for (row = 0; row < 4; row++) {
		for (col = 0; col < 4; col++) {
			printf("%8d/%d",
			       (*m)[row][col],
			       FP_EXP_POW2);
			sim_puts("b\n");
		}
		printf("\n");
	}
}
#pragma restore
#endif

fp_t fp_mul(const fp_t a, const fp_t b)
{
	fp2_t _a = a, _b = b;
	return (_a * _b) >> FP_EXP_BITS;
}

fp_t fp_div(const fp_t a, const fp_t b)
{
	fp2_t _a = a, _b = b;
	return ((_a << FP_EXP_BITS) / _b);
}

fp_t fp_add_mul(const fp_t a, const fp_t m1, const fp_t m2)
{
	return fp_add(a, fp_mul(m1, m2));
}

static fp_t _fp_cos_lookup(const fp_t v)
{
	__code const static fp_t table[9] = {
		FP_1,
		// the following values denote cos(x) * 2 ^ 32
		4212440704 >> (32 - FP_EXP_BITS),
		3968032378 >> (32 - FP_EXP_BITS),
		3571134792 >> (32 - FP_EXP_BITS),
		3037000500 >> (32 - FP_EXP_BITS),
		2386155981 >> (32 - FP_EXP_BITS),
		1643612827 >> (32 - FP_EXP_BITS),
		837906553 >> (32 - FP_EXP_BITS),
		FP_0,
	};

	if (v < FP_0) {
		return FP_1;
	};

	if (v >= FP_1) {
		return FP_0;
	}

	fp_t tmp = v * 8;
	fp_t exp = FP_EXP(tmp);
	int8_t idx = FP_INT(tmp);
	fp_t a = table[idx];

	/* The table was hit exactly */
	if (exp == 0) {
		return a;
	}

	/* No exact hit, we interpolate */
	fp_t b = table[idx + 1];

	a = fp_mul(a, fp_sub(FP_1, exp));
	a = fp_add_mul(a, table[idx + 1], exp);

	return a;
}

fp_t fp_cos(const fp_t a)
{
	fp_t tmp;
	if (a < 0)
		tmp = -a;
	else
		tmp = a;

	tmp &= (2 * FP_sPI - 1);
	if (tmp < FP_sPI/2) {
		return _fp_cos_lookup(tmp);
	}
	else if (tmp < FP_sPI) {
		return -_fp_cos_lookup(
			fp_sub(FP_sPI, tmp));
	}
	else if (tmp < 3 * FP_sPI/2) {
		return -_fp_cos_lookup(
			fp_sub(tmp, FP_sPI));
	}
	else { //  (tmp < 2 * FP_sPI)
		return _fp_cos_lookup(
			fp_sub(2 * FP_sPI, tmp));
	}
}

fp_t fp_sin(const fp_t a)
{
	return fp_cos(fp_add(a, -(FP_sPI / 2)));
}

void fp_identity_mat4x4(mat4x4_t *o) __reentrant
{
	static const mat4x4_t identity_4x4 = {
		{ FP_1, FP_0, FP_0, FP_0, },
		{ FP_0, FP_1, FP_0, FP_0, },
		{ FP_0, FP_0, FP_1, FP_0, },
		{ FP_0, FP_0, FP_0, FP_1, },
	};
	memcpy(o, &identity_4x4, sizeof(*o));
}

#define OVERLAY_FIELD fp_mul_mat4x4_mat4x4
void fp_mul_mat4x4_mat4x4(mat4x4_t *o, const mat4x4_t *a, const mat4x4_t *b) __reentrant
{
	for (pso_row = 3; pso_row >= 0; pso_row--) {
		for (pso_col = 3; pso_col >= 0; pso_col--) {
			pso_tmp = 0;
			for (pso_i = 3; pso_i >= 0; pso_i--) {
				pso_tmp = fp_add_mul(pso_tmp,
						     (*a)[pso_row][pso_i],
						     (*b)[pso_i][pso_col]);
			}
			(*o)[pso_row][pso_col] = pso_tmp;
		}
	}
}
#undef OVERLAY_FIELD

#define OVERLAY_FIELD fp_mul_mat4x4_vec4_3
void fp_mul_mat4x4_vec4_3(vec3_t *o, const mat4x4_t *a, const vec4_t *b) __reentrant
{
	for (pso_row = 0; pso_row < 3; pso_row++) {
		pso_tmp = 0;
		for (pso_i = 0; pso_i < 4; pso_i++) {
			pso_tmp = fp_add_mul(pso_tmp, (*a)[pso_row][pso_i], (*b)[pso_i]);
		}
		(*o)[pso_row] = pso_tmp;
	}
}
#undef OVERLAY_FIELD

#define OVERLAY_FIELD fp_mul_mat4x4_vec4_4
void fp_mul_mat4x4_vec4_4(vec4_t *o, const mat4x4_t *a, const vec4_t *b) __reentrant
{
	for (pso_row = 0; pso_row < 4; pso_row++) {
		pso_tmp = 0;
		for (pso_i = 0; pso_i < 4; pso_i++) {
			pso_tmp = fp_add_mul(pso_tmp, (*a)[pso_row][pso_i], (*b)[pso_i]);
		}
		(*o)[pso_row] = pso_tmp;
	}
}
#undef OVERLAY_FIELD

void fp_add_vec3_vec3(vec3_t *o, const vec3_t *a, const vec3_t *b) __reentrant
{
	int8_t i;
	for (i = 2; i >= 0; i--) {
		(*o)[i] = fp_add((*a)[i], (*b)[i]);
	}
}

void fp_add_vec4_vec4(vec4_t *o, const vec4_t *a, const vec4_t *b) __reentrant
{
	int8_t i;
	for (i = 3; i >= 0; i--) {
		(*o)[i] = fp_add((*a)[i], (*b)[i]);
	}
}

void fp_fast_add_vec3(__xdata vec3_t *o, const __xdata vec3_t *a) __reentrant
{
	int8_t i;
	for (i = 2; i >= 0; i--) {
		(*o)[i] = fp_add((*o)[i], (*a)[i]);
	}
}

#define OVERLAY_FIELD fp_pre_translate_mat4x4_vec3
void fp_pre_translate_mat4x4_vec3(mat4x4_t *m, const vec3_t *v) __reentrant
{
	/* Translate Matrix
	 *
	 * / 1 0 0 x \	 / M11 M12 M13 M14 \   / M11+x*M41 M12+x*M42 M13+x*M43 M14+x*M44 \
	 * | 0 1 0 y | * | M21 M22 M23 M24 | = | M21+y*M41 M22+y*M42 M23+y*M43 M24+y*M44 |
	 * | 0 0 1 z |	 | M31 M32 M33 M34 |   | M31+z*M41 M32+z*M42 M33+y*M43 M34+z*M44 |
	 * \ 0 0 0 1 /	 \ M41 M42 M43 M44 /   \ M41	   M42	     M43       M44	 /
	 */

	for (pso_col = 0; pso_col < 4; pso_col++) {
		pso_tmp = (*m)[3][pso_col];
		for (pso_row = 0; pso_row < 3; pso_row++) {
			(*m)[pso_row][pso_col] = fp_add_mul((*m)[pso_row][pso_col], (*v)[pso_row], pso_tmp);
		}
	}
}
#undef OVERLAY_FIELD

#define OVERLAY_FIELD fp_post_translate_mat4x4_vec3
void fp_post_translate_mat4x4_vec3(mat4x4_t *m, const vec3_t *v) __reentrant
{
	/* Translate Matrix
	 *
	 * / M11 M12 M13 M14 \	 / 1 0 0 x \   / M11 M12 M1 M11*x+M12*y+M13*z+M14 \
	 * | M21 M22 M23 M24 | * | 0 1 0 y | = | M21 M22 M2 M21*x+M22*y+M23*z+M24 |
	 * | M31 M32 M33 M34 |	 | 0 0 1 z |   | M31 M32 M3 M31*x+M32*y+M33*z+M34 |
	 * \ M41 M42 M43 M44 /	 \ 0 0 0 1 /   \ M41 M42 M4 M41*x+M42*y+M43*z+M44 /
	 */

	for (pso_row = 3; pso_row >=0; pso_row--) {
		pso_tmp = 0;
		for (pso_col = 2; pso_col >= 0; pso_col--) {
			pso_tmp = fp_add_mul(pso_tmp, (*m)[pso_row][pso_col], (*v)[pso_col]);
		}
		(*m)[pso_row][3] = fp_add((*m)[pso_row][3], pso_tmp);
	}
}
#undef OVERLAY_FIELD

#define OVERLAY_FIELD fp_pre_rotate_mat4x4_x
void fp_pre_rotate_mat4x4_x(mat4x4_t *m, fp_t a) __reentrant
{
	/* Translate Matrix
	 *
	 * /  1	 0  0  0 \   / M11 M12 M13 M14 \   / M11	 M12	     M13	 M14	     \
	 * |  0	 c -s  0 | * | M21 M22 M23 M24 | = | c*M21-s*M31 c*M22-s*M32 c*M23-s*M33 c*M24-s*M34 \
	 * |  0	 s  c  0 |   | M31 M32 M33 M34 |   | s*M21+c*M31 s*M22+c*M32 s*M23+c*M33 s*M24+c*M34 |
	 * \  0	 0  0  1 /   \ M41 M42 M43 M44 /   \ M41	 M42	     M43	 M44	     /
	 */

	xso_r2[0] = (*m)[1][0];
	xso_r2[1] = (*m)[1][1];
	xso_r2[2] = (*m)[1][2];
	xso_r2[3] = (*m)[1][3];
	xso_r3[0] = (*m)[2][0];
	xso_r3[1] = (*m)[2][1];
	xso_r3[2] = (*m)[2][2];
	xso_r3[3] = (*m)[2][3];

	pso_s = fp_sin(a);
	pso_c = fp_cos(a);
	for (pso_i = 0; pso_i < 4; pso_i++) {
		(*m)[1][pso_i] = fp_sub(
			fp_mul(pso_c, xso_r2[pso_i]),
			fp_mul(pso_s, xso_r3[pso_i]));
		(*m)[2][pso_i] = fp_add(
			fp_mul(pso_s, xso_r2[pso_i]),
			fp_mul(pso_c, xso_r3[pso_i]));
	}
}
#undef OVERLAY_FIELD

#define OVERLAY_FIELD fp_pre_rotate_mat4x4_y
void fp_pre_rotate_mat4x4_y(mat4x4_t *m, fp_t a) __reentrant
{
	/* Translate Matrix
	 *
	 * /  c	 0  s  0 \   / M11 M12 M13 M14 \   /  c*M11+s*M31  c*M12+s*M32	c*M13+s*M33  c*M14+s*M34 \
	 * |  0	 1  0  0 | * | M21 M22 M23 M24 | = |  M21	   M22		M23	     M24	 |
	 * | -s	 0  c  0 |   | M31 M32 M33 M34 |   | -s*M11+c*M31 -s*M12+c*M32 -s*M13+c*M33 -s*M14+c*M34 |
	 * \  0	 0  0  1 /   \ M41 M42 M43 M44 /   \  M41	   M42		M43	     M44	 /
	 */

	xso_r1[0] = (*m)[0][0];
	xso_r1[1] = (*m)[0][1];
	xso_r1[2] = (*m)[0][2];
	xso_r1[3] = (*m)[0][3];
	xso_r3[0] = (*m)[2][0];
	xso_r3[1] = (*m)[2][1];
	xso_r3[2] = (*m)[2][2];
	xso_r3[3] = (*m)[2][3];

	pso_s = fp_sin(a);
	pso_c = fp_cos(a);
	for (pso_i = 0; pso_i < 4; pso_i++) {
		(*m)[0][pso_i] = fp_add(
			fp_mul(pso_c, xso_r1[pso_i]),
			fp_mul(pso_s, xso_r3[pso_i]));
		(*m)[2][pso_i] = fp_sub(
			fp_mul(pso_c, xso_r3[pso_i]),
			fp_mul(pso_s, xso_r1[pso_i]));
	}
}
#undef OVERLAY_FIELD

#define OVERLAY_FIELD fp_pre_rotate_mat4x4_z
void fp_pre_rotate_mat4x4_z(mat4x4_t *m, fp_t a) __reentrant
{
	/* Translate Matrix
	 *
	 * /  c -s  0  0 \   / M11 M12 M13 M14 \   / c*M11-s*M21 c*M12-s*M22 c*M13-s*M23 c*M14-s*M24 \
	 * |  s	 c  0  0 | * | M21 M22 M23 M24 | = | s*M11+c*M21 s*M12+c*M22 s*M13+c*M23 s*M14+c*M24 |
	 * |  0	 0  1  0 |   | M31 M32 M33 M34 |   | M31	 M32	     M33	 M34	     |
	 * \  0	 0  0  1 /   \ M41 M42 M43 M44 /   \ M41	 M42	     M43	 M44	     /
	 */

	xso_r1[0] = (*m)[0][0];
	xso_r1[1] = (*m)[0][1];
	xso_r1[2] = (*m)[0][2];
	xso_r1[3] = (*m)[0][3];
	xso_r2[0] = (*m)[1][0];
	xso_r2[1] = (*m)[1][1];
	xso_r2[2] = (*m)[1][2];
	xso_r2[3] = (*m)[1][3];

	pso_s = fp_sin(a);
	pso_c = fp_cos(a);
	for (pso_i = 0; pso_i < 4; pso_i++) {
		(*m)[0][pso_i] = fp_sub(
			fp_mul(pso_c, xso_r1[pso_i]),
			fp_mul(pso_s, xso_r2[pso_i]));
		(*m)[1][pso_i] = fp_add(
			fp_mul(pso_s, xso_r1[pso_i]),
			fp_mul(pso_c, xso_r2[pso_i]));
	}
}
#undef OVERLAY_FIELD

#if WITH_FP_PRE_ROTATION
#define OVERLAY_FIELD fp_pre_rotation_mat4x4
void fp_pre_rotation_mat4x4(mat4x4_t *m, fp_t a, const vec3_t *u) __reentrant
{
	fp_identity_mat4x4(m);

	pso_s = fp_sin(a);
	pso_c = fp_cos(a);
	pso_inv_c = fp_sub(FP_1, pso_c);

	(*m)[0][0] = fp_add(
		pso_c,
		fp_mul3((*u)[0], (*u)[0], pso_inv_c));
	(*m)[0][1] = fp_sub(
		fp_mul3((*u)[0], (*u)[1], pso_inv_c),
		fp_mul((*u)[2], pso_s));
	(*m)[0][2] = fp_add(
		fp_mul3((*u)[0], (*u)[2], pso_inv_c),
		fp_mul((*u)[1], pso_s));

	(*m)[1][0] = fp_add(
		fp_mul3((*u)[1], (*u)[0], pso_inv_c),
		fp_mul((*u)[2], pso_s));
	(*m)[1][1] = fp_add(
		pso_c,
		fp_mul3((*u)[1], (*u)[1], pso_inv_c));
	(*m)[1][2] = fp_sub(
		fp_mul3((*u)[1], (*u)[2], pso_inv_c),
		fp_mul((*u)[0], pso_s));

	(*m)[2][0] = fp_sub(
		fp_mul3((*u)[2], (*u)[0], pso_inv_c),
		fp_mul((*u)[1], pso_s));
	(*m)[2][1] = fp_add(
		fp_mul3((*u)[2], (*u)[1], pso_inv_c),
		fp_mul((*u)[0], pso_s));
	(*m)[2][2] = fp_add(
		pso_c,
		fp_mul3((*u)[2], (*u)[2], pso_inv_c));
}
#undef OVERLAY_FIELD
#endif
