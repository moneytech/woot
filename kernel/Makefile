OUTFILE = woot
OBJS = boot.o main.o

LDFLAGS += -Tldscript

%.o: %.asm
	$(ASM) $(ASMFLAGS) -o $@ $<

all: $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) -o $(OUTFILE)

clean:
	rm -f $(OBJS) $(OUTFILE)

.PHONY: clean

