#ifndef KEY_H
#define KEY_H

#include <stdint.h>
#include <stdbool.h>

#define KEY_EVENT_NONE 0xff
#define KEY_MAKE_EVENT(K, E) (((uint8_t)(K)) | (((uint8_t)(E)) << 4))
#define KEY_KEY(E) (((uint8_t)(E)) & 0xf)
#define KEY_EVENT(E) ((((uint8_t)(E)) >> 4) & 0xf)

typedef enum {
	KEY_SHORT = 0,
	KEY_LONG = 1,
	KEY_PRESS = 2,
}  key_event_type_t;

void key_init(void);
void key_isr(void);
uint8_t key_consume_event();

#endif
