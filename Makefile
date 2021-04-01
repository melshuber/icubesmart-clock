TOOLCHAIN=/opt/sdcc/sdcc
CC=$(TOOLCHAIN)/bin/sdcc
SIM=$(TOOLCHAIN)/bin/s51
CFLAGS=--debug -mmcs51 --std-c99
LDFLAGS=--debug -mmcs51 --std-c99
PROG=icube.ihx
CSRC=main.c framebuffer.c cpu.c sim.c

CFLAGS+=-DSIMULATION=1 -DNOSIM_FB=1

.SUFFIXES: .rel

all: $(PROG)

$(PROG): $(CSRC:.c=.rel)
	$(CC) $(LDFLAGS) -o $@ $^

.c.rel:
	$(CC) -c $(CFLAGS) -o $@ $<

sim: all
	$(SIM) -t 51R $(PROG) -I if=xram[0xffff]

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
