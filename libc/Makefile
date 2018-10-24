LIB = libc
OBJS = crt0.o stdlib.o string.o time.o unistd.o
OBJS += asm/syscall.o

CFLAGS = -m32 -I./include
ASMFLAGS = -f elf
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

