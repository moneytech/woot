OUTFILE = woot
OBJS = boot.o main.o cpu.o gdt.o idt.o

LDFLAGS += -Tldscript
#LDFLAGS += $(LIBDIR)/string.o $(LIBDIR)/malloc.o $(LIBDIR)/stdlib.o $(LIBDIR)/arith.o

%.o: %.asm
	$(ASM) $(ASMFLAGS) -o $@ $<

all: $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) -o $(OUTFILE) -lc

clean:
	rm -f $(OBJS) $(OUTFILE)

.PHONY: clean

