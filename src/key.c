#ifdef DEBUG_KEY
#define DEBUG_MODULE key
#endif
#include "debug.h"

#include "key.h"
#include "board.h"

typedef struct {
	uint8_t cnt;
} key_state_t;

#define KEY_EVENT_QUEUE_LEN 4
static __xdata uint8_t _key_event_queue[KEY_EVENT_QUEUE_LEN];
volatile __xdata uint8_t _key_event_queue_head;
volatile __xdata uint8_t _key_event_queue_tail;

static __xdata key_state_t _key_start_state;
static __xdata key_state_t _key_cycle_state;
static __xdata key_state_t _key_next_state;

static volatile uint8_t _key_events;

void key_init(void)
{
	_key_start_state.cnt = 0;
	_key_cycle_state.cnt = 0;
	_key_next_state.cnt = 0;
	_key_event_queue_head = 7;
	_key_event_queue_tail = 7;
}

uint8_t key_consume_event()
{
	uint8_t head = _key_event_queue_head;
	uint8_t tail = _key_event_queue_tail;
	uint8_t res = KEY_EVENT_NONE;

	if (head != tail) {
		res = _key_event_queue[tail % KEY_EVENT_QUEUE_LEN];
		_key_event_queue_tail = tail + 1;
	}

	return res;
}

static void _key_emit_event(uint8_t event) __reentrant
{
	uint8_t head = _key_event_queue_head;
	uint8_t tail = _key_event_queue_tail;
	uint8_t qstate = (head ^ tail) % (2 * KEY_EVENT_QUEUE_LEN);
	if (qstate != KEY_EVENT_QUEUE_LEN) {
		_key_event_queue[head % KEY_EVENT_QUEUE_LEN] = event;
		_key_event_queue_head = head + 1;
	}
}

static void _key_update_state(uint8_t key)
{
	__xdata key_state_t *s;
	uint8_t is_up;
	switch (key) {
	case KEY_NEXT:
		s = &_key_next_state;
		is_up = KEY_NEXT_BIT;
		break;
	case KEY_CYCLE:
		s = &_key_cycle_state;
		is_up = KEY_CYCLE_BIT;
		break;
	case KEY_START:
		s = &_key_start_state;
		is_up = KEY_START_BIT;
		break;
	default:
		return;
	}

	uint8_t tmp = s->cnt;
	if (is_up) {
		if ((tmp >= KEY_SHORT_TICKS) && (tmp < KEY_LONG_TICKS)) {
			_key_emit_event(KEY_MAKE_EVENT(key, KEY_PRESS));
		}
		tmp = 0;
	} else {
		tmp++;

		switch (tmp) {
		case KEY_SHORT_TICKS:
			_key_emit_event(KEY_MAKE_EVENT(key, KEY_SHORT));
			break;
		case KEY_LONG_TICKS:
			_key_emit_event(KEY_MAKE_EVENT(key, KEY_LONG));
			break;

		}
		if (s->cnt <= KEY_LONG_TICKS) {
			s->cnt++;
		}

	}
	s->cnt = tmp;
}

void key_isr(void)
{
	_key_update_state(KEY_NEXT);
	_key_update_state(KEY_CYCLE);
	_key_update_state(KEY_START);
}
