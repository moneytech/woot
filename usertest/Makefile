OUTFILE = usertest
OBJS = main.o

all: $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) -o $(OUTFILE)

clean:
	rm -rf $(OUTFILE) $(OBJS)

.PHONY: clean

