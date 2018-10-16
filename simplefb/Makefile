MODULENAME = simplefb
OUTFILE = $(MODULENAME).ko
OBJS = main.o

CFLAGS +=
CXXFLAGS +=
LDFLAGS += -Ttext=0xC1000000 -e Initialize -shared -T../module.ldscript

all: $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) -o $(OUTFILE)

clean:
	rm -rf $(OBJS) $(OUTFILE)

.PHONY: clean

