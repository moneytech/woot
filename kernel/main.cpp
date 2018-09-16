#include <cpu.h>
#include <debugstream.h>
#include <gdt.h>
#include <idt.h>
#include <ints.h>
#include <irqs.h>
#include <multiboot.h>
#include <stdio.h>
#include <time.h>

Stream *debugStream = nullptr; // main debug stream (kernel log)

static unsigned short *video = (unsigned short *)0xC00B8000;

static bool kbdTest(Ints::State *state, void *context)
{
    byte d = _inb(0x60);
    video[1] = 0x2F00 | d;
    IRQs::SendEOI(1);
    return true;
}
static Ints::Handler kbdTestHandler = { nullptr, kbdTest, nullptr };

static uint64_t getRAMSize(multiboot_info_t *mboot_info);

extern "C" int kmain(multiboot_info_t *mbootInfo)
{
    cpuInitFPU(0x37F);
    cpuEnableSSE();
    debugStream = new DebugStream(0xE9);
    printf("[main] debugStream initialized\n");
    GDT::Initialize();
    IDT::Initialize();
    IRQs::Initialize();
    cpuEnableInterrupts();
    Time::Initialize();
    Time::StartSystemTimer();

    uint64_t ramSize = getRAMSize(mbootInfo);
    printf("[main] found %.2f MiB of usable memory\n", (double)ramSize / (double)(1 << 20));

    IRQs::RegisterHandler(1, &kbdTestHandler);
    IRQs::Enable(1);

    for(;;) cpuWaitForInterrupt(0);

    return 0xD007D007;
}

static uint64_t getRAMSize(multiboot_info_t *mboot_info)
{
    uint64_t ramSize = 0;
    for(uintptr_t mmapAddr = mboot_info->mmap_addr,
        mmapEnd = mboot_info->mmap_addr + mboot_info->mmap_length;
        mmapAddr < mmapEnd;)
    {
        multiboot_memory_map_t *mmap = (multiboot_memory_map_t *)(mmapAddr + KERNEL_BASE);
        uintptr_t blockEnd = mmap->addr + mmap->len;
        if(mmap->addr >= (1 << 20) && mmap->type == 1 && blockEnd > ramSize)
            ramSize += blockEnd;
        mmapAddr += mmap->size + 4;
    }
    return ramSize;
}
