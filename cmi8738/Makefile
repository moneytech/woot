MODULENAME = cmi8738
OUTFILE = $(MODULENAME).ko
OBJS = main.o cmi8738.o
EXTRAOBJS = ../kernel/woot

LDFLAGS += -shared -T../module.ldscript

all: $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) $(EXTRAOBJS) -o $(OUTFILE)

clean:
	rm -rf $(OBJS) $(OUTFILE)

.PHONY: clean

