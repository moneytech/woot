OUTFILE = sndplay
OBJS = main.o

CC = woot-gcc
CFLAGS = -fno-stack-protector -ggdb
LDFLAGS = -Wl,-lc,-lwoot,-T../user.ldscript
ASFLAGS =

all: $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $(OUTFILE)

clean:
	rm -rf $(OBJS) $(OUTFILE)

.PHONY: clean

