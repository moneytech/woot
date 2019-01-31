LIB = libwoot
OBJS = asm/lmemset.o
OBJS += font.o pixmap.o ui.o wm.o audio.o

LIB_STATIC = $(LIB).a
LIB_SHARED = $(LIB).so

CC = woot-gcc
CFLAGS = -ggdb -fno-stack-protector -O2
ASMFLAGS = -f elf -gdwarf2
LDFLAGS = -Wl,-lpng16,-lfreetype
SHLDFLAGS = -shared -Wl,-T../shared.ldscript,-soname,$(LIB_SHARED),-Ttext=0x00300000

%.o: %.asm
	$(ASM) $(ASMFLAGS) -o $@ $<

all: shared static

static: $(OBJS)
	$(AR) rcs $(LIB_STATIC) $(OBJS)

shared: $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $(LIB_SHARED) $(SHLDFLAGS)

clean:
	rm -rf $(LIB_STATIC) $(LIB_SHARED) $(OBJS)

.PHONY: clean

