LIB = libwoot
OBJS = wm.o

CFLAGS = -m32 -ggdb -nostdinc -I./include -I./../libc/include -Wno-expansion-to-defined
ASMFLAGS = -f elf -gdwarf2
LDFLAGS = -melf_i386 -shared

LIB_STATIC = $(LIB).a
LIB_SHARED = $(LIB).so

%.o: %.asm
	$(ASM) $(ASMFLAGS) -o $@ $<

all: shared

static: $(OBJS)
	$(AR) rcs $(LIB_STATIC) $(OBJS)

shared: $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) -o $(LIB_SHARED)

clean:
	rm -rf $(LIB_STATIC) $(LIB_SHARED) $(OBJS)

.PHONY: clean

