SUBDIRS = kernel
KERNELFILE = woot
ISODIR = $(PWD)/iso
ISOFILE = woot.iso

MAKE = make
CC = gcc
CXX = g++
ASM = yasm
LD = ld

COMMONFLAGS = -ggdb -m32 -fno-stack-protector -mno-sse -fno-pic
CFLAGS = $(COMMONFLAGS)
CXXFLAGS = $(COMMONFLAGS) -fno-exceptions
ASMFLAGS = -gdwarf2 -f elf32
LDFLAGS = -melf_i386

export MAKE
export CC
export CXX
export ASM
export LD

export CFLAGS
export CXXFLAGS
export ASMFLAGS
export LDFLAGS

all: subdirs

subdirs:
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir; \
	done

clean:
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir clean; \
	done
	rm -f $(ISODIR)/$(KERNELFILE)
	rm -f $(ISOFILE)
	
iso: all
	cp kernel/$(KERNELFILE) $(ISODIR)/$(KERNELFILE)
	grub-mkrescue -o $(ISOFILE) $(ISODIR) 2>&1

.PHONY: subdirs clean iso

