OUTFILE = simplefb.ko
OBJS = main.o

LDFLAGS += -r

all: $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) -o $(OUTFILE)

clean:
	rm -rf $(OBJS) $(OUTFILE)

.PHONY: clean

