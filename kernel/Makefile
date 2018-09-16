OUTFILE = woot
OBJS = boot.o main.o cpu.o gdt.o idt.o isrs.o ints.o irqs.o cmos.o time.o
OBJS += stream.o debugstream.o bitmap.o

LDFLAGS += -Tldscript

%.o: %.asm
	$(ASM) $(ASMFLAGS) -o $@ $<

all: $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) -o $(OUTFILE) -lc

clean:
	rm -f $(OBJS) $(OUTFILE)

.PHONY: clean

