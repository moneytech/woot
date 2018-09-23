#include <cpu.h>
#include <debugstream.h>
#include <drive.h>
#include <gdt.h>
#include <idedrive.h>
#include <idt.h>
#include <ints.h>
#include <irqs.h>
#include <multiboot.h>
#include <mutex.h>
#include <paging.h>
#include <pci.h>
#include <semaphore.h>
#include <stdio.h>
#include <thread.h>
#include <time.h>

Stream *debugStream = nullptr; // main debug stream (kernel log)

static unsigned short *video = (unsigned short *)0xC00B8000;
static Semaphore *kbdSem = nullptr;
static byte kbdData = 0;
static Mutex *vidMtx = nullptr;

static bool kbdTest(Ints::State *state, void *context)
{
    kbdData = _inb(0x60);
    video[1] = 0x2F00 | kbdData;
    kbdSem->Signal(state);
    return true;
}
static Ints::Handler kbdTestHandler = { nullptr, kbdTest, nullptr };

void kbdThread(uintptr_t arg)
{
    for(;;)
    {
        kbdSem->Wait(0, false);
        vidMtx->Acquire(0);
        printf("kbdData: %#.2x\n", kbdData);

        if(kbdData == 0x0B) // 0 press
        {
            Drive *drv = Drive::GetByIndex(0);
            byte *buf = new byte[drv->SectorSize];
            int64_t sr = drv->ReadSectors(buf, 0, 1);
            printf("sr = %d\n", sr);
            for(uint j = 0; j < drv->SectorSize; j += 16)
            {
                printf("%.8x : ", j);
                for(uint i = 0; i < 16; ++i)
                    printf("%.2x ", buf[i + j]);
                printf("| ");
                for(uint i = 0; i < 16; ++i)
                {
                    byte b = buf[i + j];
                    printf("%c", b < ' ' ? '.' : b);
                }
                printf("\n");
            }
            delete[] buf;
        }
        vidMtx->Release();
    }
}

void testThread(uintptr_t arg)
{
    Thread *ct = Thread::GetCurrent();
    for(;;)
    {
        vidMtx->Acquire(0);
        printf("test: %d\n", arg);
        vidMtx->Release();
        ct->Sleep(1000 * ct->ID, false);
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
    PCI::Initialize();
    Drive::Initialize();
    IDEDrive::Initialize();

    kbdSem = new Semaphore(0);
    vidMtx = new Mutex();

    IRQs::RegisterHandler(1, &kbdTestHandler);
    IRQs::Enable(1);

    Thread *t1 = new Thread("test 1", nullptr, (void *)testThread, 1, 0, 0, nullptr, nullptr);
    Thread *t2 = new Thread("test 2", nullptr, (void *)testThread, 2, 0, 0, nullptr, nullptr);
    Thread *t3 = new Thread("keyboard thread", nullptr, (void *)kbdThread, 0, 0, 0, nullptr, nullptr);

    t1->Enable();
    t2->Enable();
    t3->Enable();

    t1->Resume(false);
    t2->Resume(false);
    t3->Resume(false);

    Thread *ct = Thread::GetCurrent();
    for(;;)
    {
        double t = Time::GetSystemUpTime();
        Time::DateTime dt;
        Time::FracUnixToDateTime(t, &dt);
        vidMtx->Acquire(0);
        printf("main (runtime %.2d:%.2d:%.2d.%02d)\n", dt.Hour, dt.Minute, dt.Second, dt.Millisecond);
        vidMtx->Release();
        ct->Sleep(900, false);
    }

    Drive::Cleaup();
    PCI::Cleanup();

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
