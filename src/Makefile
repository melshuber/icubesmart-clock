# Defaults
TOOLCHAIN?=/opt/sdcc/sdcc
CC=$(TOOLCHAIN)/bin/sdcc
SIM=$(TOOLCHAIN)/bin/s51
TTY?=/dev/ttyUSB0
BAUD?=115200

MEMORY_MODEL=--model-medium

LDFLAGS+=-mmcs51
LDFLAGS+=--std-c99
LDFLAGS+=--debug
LDFLAGS+=$(MEMORY_MODEL)
LDFLAGS+=--iram-size 256
LDFLAGS+=--xram-size 1024
LDFLAGS+=--code-size 61440

CFLAGS=-mmcs51
CFLAGS+=--std-c99 $(MEMORY_MODEL)
CFLAGS+=--debug
CFLAGS+=-DFP_BITS_16 -DFP_EXP_BITS=8
CFLAGS+=-DUSE_FAST_ADD_VEC3_ASM=1
CFLAGS+=-DUSE_RENDER_TEX2D_ASM=1
# do not warn on unreachable code
CFLAGS+=--disable-warning 126
#CFLAGS+=-DDEBUG_LEVEL=DEBUG_DEBUG
#CFLAGS+=-DDEBUG_MAIN
#CFLAGS+=-DDEBUG_RENDER
#CFLAGS+=-DDEBUG_FRAMEBUFFER
#CFLAGS+=-DDEBUG_TIME
#CFLAGS+=-DDEBUG_FONT
#CFLAGS+=-DDEBUG_UTIL
#CFLAGS+=-DDEBUG_KEY

PROG=icube.ihx
CSRC=main.c framebuffer.c cpu.c sim.c uart.c render.c fixed-point.c time.c font.c util.c key.c

.SUFFIXES: .rel

all: $(PROG)

$(PROG): $(CSRC:.c=.rel)
	$(CC) $(LDFLAGS) -o $@ $^

.c.rel:
	$(CC) -c $(CFLAGS) -o $@ $<

sim: all
	rm -f uart_tx
	#rm -f uart_rx
	#mkfifo uart_rx
	$(SIM) -t 89C51R $(PROG) -I if=sfr[0xff] -C sim.cfg -X 24M -Sout=uart_tx,in=uart_rx

flash: all
	stcgal -P stc12 -p $(TTY) -b $(BAUD) icube.ihx

settime:
	../scripts/command-interface/set_time.sh $(TTY)

console:
	picocom -b $(BAUD) $(TTY)

clean:
	rm -f $(CSRC:.c=.rel)
	rm -f $(CSRC:.c=.sym)
	rm -f $(CSRC:.c=.asm)
	rm -f $(CSRC:.c=.adb)
	rm -f $(CSRC:.c=.rel)
	rm -f $(CSRC:.c=.lst)
	rm -f $(CSRC:.c=.rst)
	rm -f $(PROG)
	rm -f $(PROG:.ihx=.map)
	rm -f $(PROG:.ihx=.lk)
	rm -f $(PROG:.ihx=.mem)
