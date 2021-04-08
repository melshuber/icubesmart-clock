#ifndef RENDER_H
#define RENDER_H

#include "framebuffer.h"
#include "fixed-point.h"

#include <stdint.h>

typedef uint8_t tex2D_t[8];

extern const tex2D_t tex1;
extern const tex2D_t tex2;
extern const tex2D_t tex3;

void render_clear(__xdata fb_frame_t *fb);
void render_set_pixel(
	__xdata fb_frame_t *fb,
	uint8_t x,
	uint8_t y,
	uint8_t z);
void render_tex2D(
	__xdata fb_frame_t *fb,
	const __xdata tex2D_t *tex,
	mat4x4_t *transform);

void render_printfb(__xdata fb_frame_t *fb);

#endif
