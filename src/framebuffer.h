#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include "board.h"

#include <stdint.h>

typedef struct {
	uint8_t pixels[64];
} fb_frame_t;

/* logicaly private but exported to allow access for
 * fb_{front,back}_frame */

/* Index of front frame in frme buffer */
extern volatile uint8_t _fb_front_frame_idx;
/* If one the frame buffer is ready an shall be flipped */
extern volatile uint8_t _fb_back_frame_complete;

#define FB_FRAME_BUFFER_ATTR __at(0x380)
/* The two frames */
extern __xdata fb_frame_t FB_FRAME_BUFFER_ATTR _fb_frame[2];

void fb_init(void);
void fb_display_frame(void);
void fb_isr(void);

#define fb_back_frame_complete() \
	(_fb_back_frame_complete != 0)
#define fb_back_frame_completed() \
	do { _fb_back_frame_complete = 1; } while (0)
#define fb_front_frame() \
	(&_fb_frame[_fb_front_frame_idx])
#define fb_back_frame() \
	(&_fb_frame[1 - _fb_front_frame_idx])

#endif
