#ifndef DEBUG_H
#define DEBUG_H

typedef enum {
  DEBUG_DEBUG = 0,
  DEBUG_NOTE,
  DEBUG_INFO,
  DEBUG_WARN,
  DEBUG_ERROR,
  DEBUG_ALWAYS,
} debug_level;

#ifndef DEBUG_LEVEL
#define DEBUG_LEVEL DEBUG_NOTE
#endif

#define debug(...) do { dprintf(DEBUG_DEBUG, __VA_ARGS__); } while (0)
#define note(...) do { dprintf(DEBUG_NOTE, __VA_ARGS__); } while (0)
#define info(...) do { dprintf(DEBUG_INFO, __VA_ARGS__); } while (0)
#define warn(...) do { dprintf(DEBUG_WARN, __VA_ARGS__); } while (0)
#define error(...) do { dprintf(DEBUG_ERROR, __VA_ARGS__); } while (0)
#define always(...) do { dprintf(DEBUG_ERROR, __VA_ARGS__); } while (0)

#ifdef DEBUG_MODULE

#define check_debug(L) ((L) >= (DEBUG_LEVEL))
#define dprintf(L, ...)							\
	do {								\
		if (check_debug(L))					\
			printf(__VA_ARGS__);				\
	} while (0)

#else

#define check_debug(L) (0)
#define dprintf(L, ...) do {} while (0)

#endif

#endif
