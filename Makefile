ROOTDIR = $(shell pwd)
SUBDIRS = lib libc libwoot ps2mouse simplefb kernel usertest
KERNELFILE = woot
ISODIR = $(ROOTDIR)/iso
ISOFILE = woot.iso
LIBDIR = $(ROOTDIR)/lib
MOUNTPOINT = mnt
DISTFILES = usertest/usertest logo.bmp libc/libc.so

ARCH = i386
export ARCH

export LIBDIR

MAKE = make
CC = clang
CXX = clang++
ASM = yasm
LD = ld
AR = ar

COMMONFLAGS = -pipe -ggdb -m32 -fno-stack-protector -mno-sse -fno-pic -fshort-wchar
COMMONFLAGS += -I $(ROOTDIR)/include -nostdinc -ffreestanding -fno-builtin
CFLAGS = $(COMMONFLAGS)
CXXFLAGS = $(COMMONFLAGS) -fno-exceptions -fno-rtti -nostdinc++
ASMFLAGS = -gdwarf2 -f elf32 -w
LDFLAGS = -melf_i386 -nostdlib -L $(LIBDIR) --no-eh-frame-hdr

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
# your_username ALL=(ALL) NOPASSWD: /bin/mkdir
# your_username ALL=(ALL) NOPASSWD: /usr/sbin/grub-install

# sets up GRUB on hdd.img
setup-grub:
	sudo losetup -P /dev/loop1 hdd.img
	sudo mount /dev/loop1p1 $(MOUNTPOINT)
	sudo grub-install --boot-directory=$(MOUNTPOINT)/boot /dev/loop1
	sudo umount $(MOUNTPOINT)
	sudo losetup -d /dev/loop1

# builds everything and installs it on hdd image file
hdd.img: all
	sudo losetup -P /dev/loop1 hdd.img
	sudo mount /dev/loop1p1 $(MOUNTPOINT)
	-sudo cp $(ISODIR)/boot/grub/grub.cfg $(MOUNTPOINT)/boot/grub
	-sudo cp kernel/$(KERNELFILE) $(MOUNTPOINT)/
	-sudo cp modulelist $(MOUNTPOINT)/
	-sudo mkdir -p $(MOUNTPOINT)/system
	-sudo cp simplefb/simplefb.ko $(MOUNTPOINT)/system
	-sudo cp ps2mouse/ps2mouse.ko $(MOUNTPOINT)/system
	-sudo cp $(DISTFILES) $(MOUNTPOINT)
	sudo umount $(MOUNTPOINT)
	sudo losetup -d /dev/loop1

# tries to mount image file so you can make modifications to it
try-mount:
	-sudo losetup -P /dev/loop1 hdd.img
	-sudo mount /dev/loop1p1 $(MOUNTPOINT)

# tries to unmount the image (sometimes setup-grub or hdd.img
# rules don't unmount image properly; this is here to fix it)
try-unmount:
	-sudo umount $(MOUNTPOINT)
	-sudo losetup -d /dev/loop1

# alias for try-unmount
try-umount: try-unmount

.PHONY: subdirs clean distclean iso clean-img setup-grub try-mount try-unmount
