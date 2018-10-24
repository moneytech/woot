LIB = libc
OBJS = test.o stdlib.o unistd.o asm/syscall.o

CFLAGS = -m32 -I./include
ASMFLAGS = -f elf

LIB_STATIC = $(LIB).a
LIB_SHARED = $(LIB).so

%.o: %.asm
	$(ASM) $(ASMFLAGS) -o $@ $<

all: $(OBJS)
	$(AR) rcs $(LIB_STATIC) $(OBJS)

clean:
	rm -rf $(LIB_STATIC) $(LIB_SHARED) $(OBJS)

.PHONY: clean

