#include <cpu.h>
#include <debugstream.h>
#include <directoryentry.h>
#include <drive.h>
#include <ext2.h>
#include <file.h>
#include <filesystem.h>
#include <filesystemtype.h>
#include <gdt.h>
#include <idedrive.h>
#include <idt.h>
#include <ints.h>
#include <irqs.h>
#include <mbrvolume.h>
#include <multiboot.h>
#include <mutex.h>
#include <paging.h>
#include <pci.h>
#include <process.h>
#include <semaphore.h>
#include <stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stringbuilder.h>
#include <thread.h>
#include <time.h>
#include <volume.h>
#include <volumetype.h>

static unsigned short *video = (unsigned short *)0xC00B8000;
static Semaphore kbdSem(0);
static byte kbdData = 0;
static Mutex vidMtx;
static volatile bool quit = false;

static bool kbdTest(Ints::State *state, void *context)
{
    kbdData = _inb(0x60);
    video[1] = 0x2F00 | kbdData;
    kbdSem.Signal(state);
    return true;
}
static Ints::Handler kbdTestHandler = { nullptr, kbdTest, nullptr };

static void bufferDump(void *ptr, size_t n)
{
    byte *buf = (byte *)ptr;
    for(uint j = 0; j < n; j += 16)
    {
        printf("%.8x : ", j);
        for(uint i = 0; i < 16; ++i)
            printf("%.2x ", buf[i + j]);
        printf("| ");
        for(uint i = 0; i < 16; ++i)
        {
            byte b = buf[i + j];
            printf("%c", b >= ' ' && b < '\x7f' ? b : '.');
        }
        printf("\n");
    }
}

static int kbdThread(uintptr_t arg)
{
    for(; !quit;)
    {
        kbdSem.Wait(0, false);
        vidMtx.Acquire(0);
        printf("kbdData: %#.2x\n", kbdData);

        if(kbdData == 0x90) // q released
            quit = true;
        else if(kbdData == 0x93) // r released
            _outb(0x64, 0xFE);   // reset
        else if(kbdData == 0x0B) // 0 press
        {
            Drive *drv = Drive::GetByIndex(0);
            byte *buf = new byte[drv->SectorSize];
            int64_t sr = drv->ReadSectors(buf, 0, 1);
            printf("sr = %d\n", sr);
            bufferDump(buf, drv->SectorSize);
            delete[] buf;
        }
        else if(kbdData == 0x0A) // 9 press
        {
            Volume *vol = Volume::GetByIndex(0);
            byte *buf = new byte[vol->Drive->SectorSize];
            int64_t sr = vol->ReadSectors(buf, 2, 1);
            printf("sr = %d\n", sr);
            bufferDump(buf, vol->Drive->SectorSize);
            delete[] buf;
        }
        else if(kbdData == 0x09) // 8 press
        {
            if(File *f = File::Open("0:/testfile.txt", O_RDONLY))
            {
                for(byte b = 0; f->Read(&b, 1) > 0; printf("%c", b));
                delete f;
            }
        }
        else if(kbdData == 0x08) // 7 press
        {
            //if(File *f = File::Open("WOOT_OS:/boot/grub", O_DIRECTORY))
            if(File *f = File::Open("{16393ccb-173f-4c23-8f6f-0f62c6025259}:/", O_DIRECTORY))
            {
                while(DirectoryEntry *de = f->ReadDir())
                {
                    Time::DateTime mtime;
                    Time::UnixToDateTime(de->ModifyTime, &mtime);
                    printf("%-16s %s %8lu %.2d-%.2d-%.2d %.2d:%.2d\n",
                           de->Name, S_ISDIR(de->Mode) ? "<DIR> " : "      ", de->Size,
                           mtime.Year, mtime.Month, mtime.Day,
                           mtime.Hour, mtime.Minute);
                    delete de;
                }
                delete f;
            }
        }
        else if(kbdData == 0x07) // 6 press
        {
            static const char loremIpsum[] =
                    "Lorem ipsum dolor sit amet, consectetur adipiscing elit. "
                    "Donec volutpat nec neque tempus pellentesque. "
                    "Nullam sit amet tellus et nunc efficitur faucibus. "
                    "Duis egestas pretium sapien. "
                    "Phasellus molestie convallis iaculis. "
                    "Proin venenatis tellus eu quam sed.";
            if(File *f = File::Open("0:/testfile.txt", O_WRONLY))
            {
                f->Seek(6 << 10, SEEK_SET);
                for(int i = 0; i < 1; ++i)
                {
                    printf("bw: %ld\n", f->Write(loremIpsum, sizeof(loremIpsum) - 1));
                    f->Write("\n", 1);
                }
                delete f;
            }
        }
        else if(kbdData == 0x0C) // - press
        {
            if(File *f = File::Open("0:/", O_DIRECTORY))
            {
                printf("file delete result: %d\n", f->Remove("testfile2.txt"));
                delete f;
            }
        }
        else if(kbdData == 0x0D) // = press
        {
            if(File *f = File::Open("0:/", O_DIRECTORY))
            {
                bool ok = f->Create("testfile2.txt", S_IFREG | 0664);
                //bool ok = f->Create("newdir", S_IFDIR | 0755);
                printf("create %s\n", ok ? "success" : "fail");
                delete f;
            }
        }
        vidMtx.Release();
    }
    return 0x11223344;
}

int testThread(uintptr_t arg)
{
    Thread *ct = Thread::GetCurrent();
    for(; !quit;)
    {
        vidMtx.Acquire(0);
        printf("test: %d\n", arg);
        vidMtx.Release();
        ct->Sleep(1000 * ct->ID, false);
    }
    return 0x11 * arg;
}

static uint64_t getRAMSize(multiboot_info_t *mboot_info);

extern "C" int kmain(multiboot_info_t *mbootInfo)
{
    srand(time(nullptr));
    printf("[main] Starting woot...\n");
    printf("[main] Kernel version: v%d.%d %s\n",
           KERNEL_VERSION_MAJOR,
           KERNEL_VERSION_MINOR,
           KERNEL_VERSION_DESCRIPTION);
    GDT::Initialize();
    IDT::Initialize();

    uint64_t ramSize = getRAMSize(mbootInfo);
    printf("[main] Found %.2f MiB of usable memory\n", (double)ramSize / (double)(1 << 20));

    Paging::Initialize(ramSize);
    Thread::Initialize();
    Process::Initialize();
    IRQs::Initialize();
    cpuEnableInterrupts();
    Time::Initialize();
    Time::StartSystemTimer();
    PCI::Initialize();
    Drive::Initialize();
    IDEDrive::Initialize();
    VolumeType::Initialize();
    Volume::Initialize();
    FileSystemType::Initialize();
    FileSystem::Initialize();

    VolumeType::Add(new MBRVolumeType());
    VolumeType::AutoDetect();
    FileSystemType::Add(new EXT2FileSystemType());
    FileSystemType::AutoDetect();

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
    for(; !quit;)
    {
        double t = Time::GetSystemUpTime();
        Time::DateTime dt;
        Time::FracUnixToDateTime(t, &dt);
        vidMtx.Acquire(0);
        printf("[main] runtime %.2d:%.2d:%.2d.%02d\n", dt.Hour, dt.Minute, dt.Second, dt.Millisecond);
        vidMtx.Release();
        ct->Sleep(900, false);
    }
    printf("[main] Closing system\n");


    t1->Finished->Wait(0, false);
    t2->Finished->Wait(0, false);
    t3->Finished->Wait(0, false);

    delete t1;
    delete t2;
    delete t3;

    FileSystem::SynchronizeAll();
    Volume::FlushAll();
    Volume::Cleanup();
    Drive::Cleanup();
    PCI::Cleanup();

    printf("[main] System closed.\n");
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
