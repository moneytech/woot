LIB = libc
OBJS = crt0.o
OBJS += ctype.o fcntl.o malloc.o math.o softdiv.o stdio.o stdlib.o string.o time.o unistd.o
OBJS += asm/syscall.o

CFLAGS = -m32 -ggdb -nostdinc -I./include -Wno-expansion-to-defined
ASMFLAGS = -f elf -gdwarf2
LDFLAGS = -melf_i386 -shared

LIB_STATIC = $(LIB).a
LIB_SHARED = $(LIB).so

%.o: %.asm
	$(ASM) $(ASMFLAGS) -o $@ $<

all: $(OBJS)
	$(AR) rcs $(LIB_STATIC) $(OBJS)

shared: $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) -o $(LIB_SHARED)

clean:
	rm -rf $(LIB_STATIC) $(LIB_SHARED) $(OBJS)

.PHONY: clean

