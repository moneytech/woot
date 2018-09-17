#include <cpu.h>
#include <debugstream.h>
#include <gdt.h>
#include <idt.h>
#include <ints.h>
#include <irqs.h>
#include <multiboot.h>
#include <paging.h>
#include <stdio.h>
#include <thread.h>
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

void testThread(uintptr_t arg)
{
    Thread *ct = Thread::GetCurrent();
    for(;;)
    {
        printf("test: %d\n", arg);
        ct->Sleep(400 + 150 * ct->ID, false);
    }
}

static uint64_t getRAMSize(multiboot_info_t *mboot_info);

extern "C" int kmain(multiboot_info_t *mbootInfo)
{
    cpuInitFPU(0x37F);
    cpuEnableSSE();
    debugStream = new DebugStream(0xE9);
    printf("[main] debugStream initialized\n");
    GDT::Initialize();
    IDT::Initialize();

    uint64_t ramSize = getRAMSize(mbootInfo);
    printf("[main] found %.2f MiB of usable memory\n", (double)ramSize / (double)(1 << 20));

    Paging::Initialize(ramSize);
    Thread::Initialize();
    IRQs::Initialize();
    cpuEnableInterrupts();
    Time::Initialize();
    Time::StartSystemTimer();

    IRQs::RegisterHandler(1, &kbdTestHandler);
    IRQs::Enable(1);

    Thread *t1 = new Thread("test 1", nullptr, (void *)testThread, 1, 0, 0, nullptr, nullptr);
    Thread *t2 = new Thread("test 2", nullptr, (void *)testThread, 2, 0, 0, nullptr, nullptr);

    t1->Enable();
    t2->Enable();

    t1->Resume(false);
    t2->Resume(false);

    Thread *ct = Thread::GetCurrent();
    for(;;)
    {
        printf("main\n");
        ct->Sleep(250, false);
    }

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
