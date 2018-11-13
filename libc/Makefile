LIB = libc
CRT0 = crt0.o
OBJS = ctype.o fcntl.o inttypes.o libgen.o malloc.o math.o softdiv.o
OBJS += stat.o stdio.o stdlib.o string.o strings.o time.o unistd.o
OBJS += asm/syscall.o

CFLAGS = -m32 -ggdb -nostdinc -I./include -Wno-expansion-to-defined -fpic
ASMFLAGS = -f elf -gdwarf2
LDFLAGS = -melf_i386 -shared -Ttext 0x01000000

LIB_STATIC = $(LIB).a
LIB_SHARED = $(LIB).so

%.o: %.asm
	$(ASM) $(ASMFLAGS) -o $@ $<

all: static

static: $(OBJS) $(CRT0)
	$(AR) rcs $(LIB_STATIC) $(OBJS)

shared: $(OBJS) $(CRT0)
	$(LD) $(LDFLAGS) $(OBJS) -o $(LIB_SHARED)

clean:
	rm -rf $(LIB_STATIC) $(LIB_SHARED) $(OBJS) $(CRT0)

.PHONY: clean

