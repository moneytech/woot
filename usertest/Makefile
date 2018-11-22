OUTFILE = usertest
OBJS = main.o

COMMONFLAGS = -m32 -ggdb -nostdinc -mno-sse
COMMONFLAGS += -I../libc/include -I../libwoot/include -I../zlib/include -I../libpng/include
CFLAGS = $(COMMONFLAGS)
CXXFLAGS = $(COMMONFLAGS)
LDFLAGS = -melf_i386 -nostdlib -shared -Bdynamic --no-allow-shlib-undefined
LDFLAGS += -L../libc -L../libwoot -L../zlib/lib -L../libpng/lib -T../user.ldscript
LIBS = -lc -lwoot -lz -lpng
CRT0 = ../libc/crt0.o

all: $(OBJS)
	$(LD) $(LDFLAGS) $(CRT0) $(OBJS) -o $(OUTFILE) $(LIBS)

clean:
	rm -rf $(OUTFILE) $(OBJS)

.PHONY: clean

