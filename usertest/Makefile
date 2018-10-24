OUTFILE = usertest
OBJS = main.o

COMMONFLAGS = -m32 -nostdinc -I../libc/include
CFLAGS = $(COMMONFLAGS)
CXXFLAGS = $(COMMONFLAGS)
LDFLAGS = -melf_i386 -nostdlib -L../libc
LIBS = -lc

all: $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) -o $(OUTFILE) $(LIBS)

clean:
	rm -rf $(OUTFILE) $(OBJS)

.PHONY: clean

