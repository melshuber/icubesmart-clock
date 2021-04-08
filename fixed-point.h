#ifndef FIXED_POINT_H
#define FIXED_POINT_H

#ifndef WITH_FP_ROTATION
#define WITH_FP_ROTATION 0
#endif

#define USE_ADD_VEC3_ASM 1

#include <stdio.h>
#include <stdint.h>

#ifdef FP_BITS_8
#define FP_BIT 8
typedef int8_t fp_t;
typedef int16_t fp2_t;
#elif FP_BITS_16
#define FP_BIT 16
typedef int16_t fp_t;
typedef int32_t fp2_t;
#elif FP_BITS_32
#define FP_BIT 32
typedef int32_t fp_t;
typedef int64_t fp2_t;
#else
/* Set Defaults */
#define FP_BITS_8
#define FP_BIT 8
typedef int8_t fp_t;
typedef int16_t fp2_t;
#endif

#ifndef FP_EXP_BITS
#define FP_EXP_BITS (FP_BIT / 2)
#endif

#if FP_EXP_BITS == 0
#define FP_EXP_POW2 0x0001
#elif FP_EXP_BITS == 1
#define FP_EXP_POW2 0x0002
#elif FP_EXP_BITS == 2
#define FP_EXP_POW2 0x0004
#elif FP_EXP_BITS == 3
#define FP_EXP_POW2 0x0008
#elif FP_EXP_BITS == 4
#define FP_EXP_POW2 0x0010
#elif FP_EXP_BITS == 5
#define FP_EXP_POW2 0x0020
#elif FP_EXP_BITS == 6
#define FP_EXP_POW2 0x0040
#elif FP_EXP_BITS == 7
#define FP_EXP_POW2 0x0080
#elif FP_EXP_BITS == 8
#define FP_EXP_POW2 0x0100
#elif FP_EXP_BITS == 9
#define FP_EXP_POW2 0x0200
#elif FP_EXP_BITS == 10
#define FP_EXP_POW2 0x0400
#elif FP_EXP_BITS == 11
#define FP_EXP_POW2 0x0800
#elif FP_EXP_BITS == 12
#define FP_EXP_POW2 0x1000
#elif FP_EXP_BITS == 13
#define FP_EXP_POW2 0x2000
#elif FP_EXP_BITS == 14
#define FP_EXP_POW2 0x4000
#elif FP_EXP_BITS == 15
#define FP_EXP_POW2 0x8000
#else
#error unsupported FP_EXP_BITS
#endif

#define FP_INT_BITS ((FP_BIT - 1) - FP_EXP_BITS)
#define FP_FROM_INT(X) ((X) << FP_EXP_BITS)
#define FP_0 FP_FROM_INT(0)
#define FP_1 FP_FROM_INT(1)
#define FP_2 FP_FROM_INT(2)
#define FP_3 FP_FROM_INT(3)
#define FP_4 FP_FROM_INT(4)
#define FP_m1 FP_FROM_INT(-1)
#define FP_m2 FP_FROM_INT(-2)
#define FP_m3 FP_FROM_INT(-3)
#define FP_m4 FP_FROM_INT(-4)
#define FP_INT(X) ((X) >> FP_EXP_BITS)
#define FP_EXP(X) ((X) & (FP_EXP_POW2 - 1))

/* sPI is a scaled version of PI: The trig functions use values
 * relative to pi as input for the computations. "defining" PI as a
 * power of two greatly eases detection of the quadrant.
 *
 * Note 1:
 * - fp_sin(0) = fp(sin(2)) = 0
 * - fp_sin(1) = 1
 * - fp_sin(3) = -1
 *
 * Note 2: trigonometry functions relay on FP_sPI beeing 2
 */
#define FP_sPI FP_FROM_INT(2)

#define fp_add(A, B) ((A) + (B))
#define fp_sub(A, B) ((A) - (B))
fp_t fp_mul(const fp_t a, const fp_t b);
fp_t fp_add_mul(const fp_t a, const fp_t m1, const fp_t m2);
fp_t fp_div(const fp_t a, const fp_t b);
fp_t fp_cos(const fp_t a);
fp_t fp_sin(const fp_t a);

#define fp_mul3(A, B, C) fp_mul((A), fp_mul(B, C))

// matrix defined as [line][col]
typedef fp_t mat4x4_t[4][4];
typedef fp_t vec3_t[3];
typedef fp_t vec4_t[4];

void fp_print_vec3(const vec3_t *v);
void fp_print_vec4(const vec4_t *v);
void fp_print_mat4x4(const mat4x4_t *m);

/* These functions are relatively slow and consume quite a bit of
 * memory. To avoid spilling DSEG and PSEG we define them reentrant
 * thus they use the stack for locals. */
void fp_identity_mat4x4(mat4x4_t *o) __reentrant;
void fp_add_vec3_vec3(vec3_t *o, const vec3_t *a, const vec3_t *b) __reentrant;
void fp_add_vec4_vec4(vec4_t *o, const vec4_t *a, const vec4_t *b) __reentrant;
void fp_fast_add_vec3(__xdata vec3_t *o, const __xdata vec3_t *a) __reentrant;

void fp_mul_mat4x4_mat4x4(mat4x4_t *o, const mat4x4_t *a, const mat4x4_t *b) __reentrant;
void fp_mul_mat4x4_vec4(vec3_t *o, const mat4x4_t *a, const vec4_t *b) __reentrant;
void fp_mul_mat4x4_vec4_3(vec3_t *o, const mat4x4_t *a, const vec4_t *b) __reentrant;
void fp_mul_mat4x4_vec4_4(vec4_t *o, const mat4x4_t *a, const vec4_t *b) __reentrant;

void fp_pre_translate_mat4x4_vec3(mat4x4_t *m, const vec3_t *v) __reentrant;
void fp_post_translate_mat4x4_vec3(mat4x4_t *m, const vec3_t *v) __reentrant;
void fp_pre_rotate_mat4x4_x(mat4x4_t *m, fp_t a) __reentrant;
void fp_pre_rotate_mat4x4_y(mat4x4_t *m, fp_t a) __reentrant;
void fp_pre_rotate_mat4x4_z(mat4x4_t *m, fp_t a) __reentrant;

#if WITH_FP_ROTATION
void fp_rotation_mat4x4(mat4x4_t *m, fp_t a, const vec3_t *u) __reentrant;
#endif

typedef union { vec4_t v4; vec3_t v3; } u_vec4_t;

extern const u_vec4_t unit_x;
extern const u_vec4_t unit_y;
extern const u_vec4_t unit_z;
extern const u_vec4_t unit_mx;
extern const u_vec4_t unit_my;
extern const u_vec4_t unit_mz;

void fp_add_vec3_vec3_2_xseg(__xdata vec3_t *o, const __xdata vec3_t *a) __reentrant;

#endif
