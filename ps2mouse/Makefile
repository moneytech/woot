MODULENAME = ps2mouse
OUTFILE = $(MODULENAME).ko
OBJS = main.o ps2mouse.o
EXTRAOBJS = ../kernel/woot

LDFLAGS += -Ttext=0xC8010000 -e Initialize -shared -T../module.ldscript

all: $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) $(EXTRAOBJS) -o $(OUTFILE)

clean:
	rm -rf $(OBJS) $(OUTFILE)

.PHONY: clean

