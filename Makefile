ROOTDIR = $(shell pwd)
SUBDIRS = lib kernel
KERNELFILE = woot
ISODIR = $(ROOTDIR)/iso
ISOFILE = woot.iso
LIBDIR = $(ROOTDIR)/lib

export LIBDIR

MAKE = make
CC = clang
CXX = clang++
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

distclean: clean
	rm -f $(ISOFILE) hdd.img

# builds iso image
iso: all
	cp kernel/$(KERNELFILE) $(ISODIR)/$(KERNELFILE)
	grub-mkrescue -o $(ISOFILE) $(ISODIR) 2>&1

# recreates empty hdd image from archive (in case it gets borked)
clean-img:
	gunzip hdd-empty-ext2.img.gz -c > hdd.img
	
# you may need to add these lines below to your sudoers
# file in order to make next four rules work

# your_username ALL=(ALL) NOPASSWD: /bin/mount
# your_username ALL=(ALL) NOPASSWD: /bin/umount
# your_username ALL=(ALL) NOPASSWD: /sbin/losetup
# your_username ALL=(ALL) NOPASSWD: /bin/cp
# your_username ALL=(ALL) NOPASSWD: /usr/sbin/grub-install

# sets up GRUB on hdd.img
setup-grub:
	sudo losetup -P /dev/loop1 hdd.img
	sudo mount /dev/loop1p1 mnt
	sudo grub-install --boot-directory=mnt/boot /dev/loop1
	sudo umount mnt
	sudo losetup -d /dev/loop1

# builds everything and installs it on hdd image file
hdd.img: all
	sudo losetup -P /dev/loop1 hdd.img
	sudo mount /dev/loop1p1 mnt
	-sudo cp $(ISODIR)/boot/grub/grub.cfg mnt/boot/grub
	-sudo cp kernel/$(KERNELFILE) mnt/
	sudo umount mnt
	sudo losetup -d /dev/loop1

# tries to mount image file so you can make modifications to it
try-mount:
	-sudo losetup -P /dev/loop1 hdd.img
	-sudo mount /dev/loop1p1 mnt

# tries to unmount the image (sometimes setup-grub or hdd.img
# rules don't unmount image properly; this is here to fix it)
try-unmount:
	-sudo umount mnt
	-sudo losetup -d /dev/loop1

.PHONY: subdirs clean distclean iso clean-img setup-grub try-mount try-unmount
