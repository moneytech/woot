ROOTDIR = $(shell pwd)
SUBDIRS = lib kernel
KERNELFILE = woot
ISODIR = $(ROOTDIR)/iso
ISOFILE = woot.iso
LIBDIR = $(ROOTDIR)/lib

export LIBDIR

MAKE = make
CC = gcc
CXX = g++
ASM = yasm
LD = ld
AR = ar

COMMONFLAGS = -ggdb -m32 -fno-stack-protector -mno-sse -fno-pic -fshort-wchar -nostdinc
COMMONFLAGS += -I $(ROOTDIR)/include
CFLAGS = $(COMMONFLAGS)
CXXFLAGS = $(COMMONFLAGS) -fno-exceptions -fno-rtti
ASMFLAGS = -gdwarf2 -f elf32
LDFLAGS = -melf_i386 -nostdlib -L $(LIBDIR)

export MAKE
export CC
export CXX
export ASM
export LD
export AR

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

