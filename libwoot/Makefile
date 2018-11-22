LIB = libwoot
OBJS = asm/lmemset.o
OBJS += pixmap.o wm.o

LIB_STATIC = $(LIB).a
LIB_SHARED = $(LIB).so

CFLAGS = -m32 -ggdb -nostdinc -I./include -I./../libc/include -Wno-expansion-to-defined
ASMFLAGS = -f elf -gdwarf2
LDFLAGS = -melf_i386
SHLDFLAGS = -shared -T../shared.ldscript -soname $(LIB_SHARED)

%.o: %.asm
	$(ASM) $(ASMFLAGS) -o $@ $<

all: shared static

static: $(OBJS)
	$(AR) rcs $(LIB_STATIC) $(OBJS)

shared: $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) -o $(LIB_SHARED) $(SHLDFLAGS)

clean:
	rm -rf $(LIB_STATIC) $(LIB_SHARED) $(OBJS)

.PHONY: clean

