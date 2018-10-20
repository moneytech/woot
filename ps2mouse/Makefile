MODULENAME = ps2mouse
OUTFILE = $(MODULENAME).ko
OBJS = main.o ps2mouse.o

CFLAGS += -I.
CXXFLAGS += -I.
LDFLAGS += -Ttext=0xC1080000 -e Initialize -shared -T../module.ldscript

all: $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) -o $(OUTFILE)

clean:
	rm -rf $(OBJS) $(OUTFILE)

.PHONY: clean

