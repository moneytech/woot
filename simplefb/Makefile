MODULENAME = simplefb
OUTFILE = $(MODULENAME).ko
OBJS = main.o simplefb.o
EXTRAOBJS = ../kernel/woot

LDFLAGS += -shared -T../module.ldscript

all: $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) $(EXTRAOBJS) -o $(OUTFILE)

clean:
	rm -rf $(OBJS) $(OUTFILE)

.PHONY: clean

