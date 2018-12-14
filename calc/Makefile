OUTFILE = calc
OBJS = main.o

CC = woot-gcc
CFLAGS = -ggdb -fno-stack-protector
LDFLAGS = -Wl,-lc,-lwoot,-lgmp

all: $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $(OUTFILE)

clean:
	rm -rf $(OBJS) $(OUTFILE)

.PHONY: clean

