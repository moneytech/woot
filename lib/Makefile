LIBCOBJS = malloc.o string.o stdlib.o arith.o math.o cxx.o stdio.o sched.o

all: $(LIBCOBJS)
	$(AR) r libc.a $(LIBCOBJS)

clean:
	rm -f $(LIBCOBJS) libc.a

.PHONY: clean

