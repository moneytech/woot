OUTFILE = woot
MAPFILE = woot.map
OBJS = boot.o main.o cpu.o gdt.o idt.o isrs.o ints.o irqs.o cmos.o time.o
OBJS += stream.o debugstream.o bitmap.o paging.o thread.o process.o
OBJS += objectqueue.o semaphore.o miscasm.o pci.o drive.o idedrive.o
OBJS += mutex.o volume.o volumetype.o partitionvolume.o bufferedvolume.o
OBJS += filesystem.o filesystemtype.o ext2.o inode.o file.o dentry.o
OBJS += tokenizer.o directoryentry.o uuid.o memorystream.o stringbuilder.o
OBJS += inputdevice.o ps2keyboard.o elf.o filestream.o framebuffer.o fbfont.o
OBJS += syscalls.o kwm.o pixmap.o cdfs.o usbcontroller.o usbdevice.o uhcicontroller.o

LDFLAGS += -Tldscript -shared -Bsymbolic

%.o: %.asm
	$(ASM) $(ASMFLAGS) -o $@ $<

all: $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) -o $(OUTFILE) -lc -Map $(MAPFILE)

clean:
	rm -f $(OBJS) $(OUTFILE) $(MAPFILE)

.PHONY: clean

