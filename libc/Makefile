LIB = libc

CRT0 = crt0.o

OBJS = ctype.o errno.o fcntl.o init.o inttypes.o libgen.o malloc.o math.o signal.o
OBJS += softdiv.o stat.o stdio.o stdlib.o string.o strings.o time.o unistd.o

OBJS += asm/cos.o asm/exp.o asm/fmod.o asm/frexp.o asm/log.o asm/setjmp.o
OBJS += asm/sin.o asm/sqrt.o asm/syscall.o

LIB_STATIC = $(LIB).a
LIB_SHARED = $(LIB).so

CFLAGS = -m32 -ggdb -nostdinc -I./include -Wno-expansion-to-defined -fno-stack-protector
ASMFLAGS = -f elf -gdwarf2
LDFLAGS = -melf_i386
SHLDFLAGS = -shared -T../shared.ldscript -soname $(LIB_SHARED) -Ttext=0x00800000 # setting unique base makes debugging easier

%.o: %.asm
	$(ASM) $(ASMFLAGS) -o $@ $<

all: shared static

static: $(OBJS) $(CRT0)
	$(AR) rcs $(LIB_STATIC) $(OBJS)

shared: $(OBJS) $(CRT0)
	$(LD) $(LDFLAGS) $(OBJS) -o $(LIB_SHARED) $(SHLDFLAGS)

clean:
	rm -rf $(LIB_STATIC) $(LIB_SHARED) $(OBJS) $(CRT0)

.PHONY: clean

