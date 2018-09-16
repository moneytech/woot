LIBCOBJS = malloc.o string.o stdlib.o arith.o

all: $(LIBCOBJS)
	$(AR) r libc.a $(LIBCOBJS)

clean:
	rm -f $(LIBCOBJS) libc.a

.PHONY: clean

