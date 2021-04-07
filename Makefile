TOOLCHAIN=/opt/sdcc/sdcc
CC=$(TOOLCHAIN)/bin/sdcc
SIM=$(TOOLCHAIN)/bin/s51
MEMORY_MODEL=--model-medium
CFLAGS=--debug -mmcs51 --std-c99 $(MEMORY_MODEL) -DFP_BITS_16 -DFP_EXP_BITS=8
LDFLAGS=--debug -mmcs51 --std-c99 $(MEMORY_MODEL)
PROG=icube.ihx
CSRC=main.c framebuffer.c cpu.c sim.c uart.c fixed-point.c

CFLAGS+=-DSIMULATION=1 -DNOSIM_FB=1 -DNOSIM_UART=0

.SUFFIXES: .rel

all: $(PROG)

$(PROG): $(CSRC:.c=.rel)
	$(CC) $(LDFLAGS) -o $@ $^

.c.rel:
	$(CC) -c $(CFLAGS) -o $@ $<

sim: all
	rm -f uart_rx
	rm -f uart_tx
	mkfifo uart_rx
	$(SIM) -t 89C51R $(PROG) -I if=xram[0xffff] -C sim.cfg -X 12M -Sout=uart_tx,in=uart_rx

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
