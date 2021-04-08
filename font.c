#ifdef DEBUG_FONT
#define DEBUG_MODULE font
#endif
#include "debug.h"

#include "render.h"
#include "font.h"

static const __code tex2D_t _font_none =
{              /* 8 4 2 1 8 4 2 1 */
	0x00,  /*                 */
	0x00,  /*                 */
	0x18,  /*       X X       */
	0x3C,  /*     X X X X     */
	0x3C,  /*     X X X X     */
	0x18,  /*       X X       */
	0x00,  /*                 */
	0x00,  /*                 */
};

static const __code tex2D_t _font_digits[10] =
{
	{              /* 8 4 2 1 8 4 2 1 */
		0x3C,  /*     X X X X     */
		0x66,  /*   X X     X X   */
		0x66,  /*   X X     X X   */
		0x66,  /*   X X     X X   */
		0x66,  /*   X X     X X   */
		0x66,  /*   X X     X X   */
		0x66,  /*   X X     X X   */
		0x3C,  /*     X X X X     */
	},
	{              /* 8 4 2 1 8 4 2 1 */
		0x18,  /*       X X       */
		0x38,  /*     X X X       */
		0x18,  /*       X X       */
		0x18,  /*       X X       */
		0x18,  /*       X X       */
		0x18,  /*       X X       */
		0x18,  /*       X X       */
		0x3C,  /*     X X X X     */
	},
	{              /* 8 4 2 1 8 4 2 1 */
		0x3C,  /*     X X X X     */
		0x66,  /*   X X     X X   */
		0x06,  /*           X X   */
		0x06,  /*         X X     */
		0x0C,  /*       X X       */
		0x18,  /*     X X         */
		0x30,  /*   X X           */
		0x7E,  /*   X X X X X X   */
	},
	{              /* 8 4 2 1 8 4 2 1 */
		0x3C,  /*     X X X X     */
		0x66,  /*   X X     X X   */
		0x06,  /*           X X   */
		0x1C,  /*       X X X     */
		0x06,  /*           X X   */
		0x06,  /*           X X   */
		0x66,  /*   X X     X X   */
		0x3C,  /*     X X X X     */
	},
	{              /* 8 4 2 1 8 4 2 1 */
		0x0C,  /*         X X     */
		0x18,  /*       X X       */
		0x30,  /*     X X         */
		0x60,  /*   X X           */
		0x6C,  /*   X X   X X     */
		0x7E,  /*   X X X X X X   */
		0x0C,  /*         X X     */
		0x0C,  /*         X X     */
	},
	{              /* 8 4 2 1 8 4 2 1 */
		0x7E,  /*   X X X X X X   */
		0x60,  /*   X X           */
		0x60,  /*   X X           */
		0x7C,  /*   X X X X X     */
		0x06,  /*           X X   */
		0x06,  /*           X X   */
		0x66,  /*   X X     X X   */
		0x3c,  /*     X X X X     */
	},
	{              /* 8 4 2 1 8 4 2 1 */
		0x3C,  /*     X X X X     */
		0x66,  /*   X X     X X   */
		0x60,  /*   X X           */
		0x7C,  /*   X X X X X     */
		0x66,  /*   X X     X X   */
		0x66,  /*   X X     X X   */
		0x66,  /*   X X     X X   */
		0x3C,  /*     X X X X     */
	},
	{              /* 8 4 2 1 8 4 2 1 */
		0x7E,  /*   X X X X X X   */
		0x06,  /*           X X   */
		0x06,  /*           X X   */
		0x0C,  /*         X X     */
		0x18,  /*       X X       */
		0x18,  /*       X X       */
		0x18,  /*       X X       */
		0x18,  /*       X X       */
	},
	{              /* 8 4 2 1 8 4 2 1 */
		0x3C,  /*     X X X X     */
		0x66,  /*   X X     X X   */
		0x66,  /*   X X     X X   */
		0x3C,  /*     X X X X     */
		0x66,  /*   X X     X X   */
		0x66,  /*   X X     X X   */
		0x66,  /*   X X     X X   */
		0x3C,  /*     X X X X     */
	},
	{              /* 8 4 2 1 8 4 2 1 */
		0x3C,  /*     X X X X     */
		0x66,  /*   X X     X X   */
		0x66,  /*   X X     X X   */
		0x66,  /*   X X     X X   */
		0x3E,  /*     X X X X X   */
		0x06,  /*           X X   */
		0x66,  /*   X X     X X   */
		0x3C,  /*     X X X X     */
	},
};

const __code tex2D_t *font_get_texture(char c)
{
	debug("font_get_texture: '%c'/%x\n", c);
	if ((c >= '0') && (c <= '9')) {
		return &_font_digits[c - '0'];
	}
	warn("font_get_texture: '%c'/%x NOT FOUND\n", c, c);
	return &_font_none;
}
