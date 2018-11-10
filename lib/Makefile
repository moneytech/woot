LIBCOBJS = malloc.o string.o stdlib.o arith.o math.o cxx.o stdio.o sched.o

ifeq ($(ARCH), i386)
	LIBCOBJS += i386/memmove.o i386/memset.o
endif


all: $(LIBCOBJS)
	$(AR) r libc.a $(LIBCOBJS)

%.o: %.asm
	$(ASM) $(ASMFLAGS) -o $@ $<

clean:
	rm -f $(LIBCOBJS) libc.a

.PHONY: clean

