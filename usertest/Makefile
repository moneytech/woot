OUTFILE = usertest
OBJS = main.o

COMMONFLAGS = -m32 -ggdb -nostdinc -I../libc/include
CFLAGS = $(COMMONFLAGS)
CXXFLAGS = $(COMMONFLAGS)
LDFLAGS = -melf_i386 -nostdlib -L../libc
LIBS = -lc
CRT0 = ../libc/crt0.o

all: $(OBJS)
	$(LD) $(LDFLAGS) $(CRT0) $(OBJS) -o $(OUTFILE) $(LIBS)

clean:
	rm -rf $(OUTFILE) $(OBJS)

.PHONY: clean

