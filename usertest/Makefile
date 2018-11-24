OUTFILE = usertest
OBJS = main.o

CC = woot-gcc
COMMONFLAGS = -ggdb -fno-stack-protector
CFLAGS = $(COMMONFLAGS)
CXXFLAGS = $(COMMONFLAGS)
LDFLAGS =
LIBS = -lc -lwoot -lz -lpng -lfreetype

all: $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $(OUTFILE) $(LIBS)

clean:
	rm -rf $(OUTFILE) $(OBJS)

.PHONY: clean

