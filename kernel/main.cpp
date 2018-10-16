#include <cpu.h>
#include <debugstream.h>
#include <dentry.h>
#include <directoryentry.h>
#include <drive.h>
#include <elf.h>
#include <ext2.h>
#include <file.h>
#include <filestream.h>
#include <filesystem.h>
#include <filesystemtype.h>
#include <framebuffer.h>
#include <gdt.h>
#include <idedrive.h>
#include <idt.h>
#include <inputdevice.h>
#include <ints.h>
#include <irqs.h>
#include <mbrvolume.h>
#include <multiboot.h>
#include <mutex.h>
#include <paging.h>
#include <pci.h>
#include <process.h>
#include <ps2keyboard.h>
#include <semaphore.h>
#include <stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stringbuilder.h>
#include <thread.h>
#include <tokenizer.h>
#include <time.h>
#include <volume.h>
#include <volumetype.h>

multiboot_info_t *MultibootInfo = nullptr;

extern DebugStream debugStream;
static unsigned short *video = (unsigned short *)0xC00B8000;
static Semaphore kbdSem(0);
static byte kbdData = 0;
static volatile bool quit = false;

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

int testThread(uintptr_t arg)
{
    Thread *ct = Thread::GetCurrent();
    for(; !quit;)
    {
        //printf("test: %d\n", arg);
        ct->Sleep(100 * ct->ID, false);
    }
    return 0x11 * arg;
}

static uint64_t getRAMSize(multiboot_info_t *mboot_info);

static char vkToChar(VirtualKey vk, bool shift, bool caps, bool num)
{
    static const char *digits = "0123456789";
    static const char *shiftDigits = ")!@#$%^&*(";
    static const char *lowerLetters = "abcdefghijklmnopqrstuvwxyz";
    static const char *upperLetters = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

    unsigned int k = (unsigned int)vk;

    if(k >= (unsigned int)VirtualKey::NumPad0 && k <= (unsigned int)VirtualKey::NumPad9)
    {
        if(num)
            return digits[k - (unsigned int)VirtualKey::NumPad0];
    }
    else if(k >= (unsigned int)VirtualKey::Key0 && k <= (unsigned int)VirtualKey::Key9)
    {
        unsigned int dig = k - (unsigned int)VirtualKey::Key0;
        return shift ? shiftDigits[dig] : digits[dig];
    }
    else if(k >= (unsigned int)VirtualKey::KeyA && k <= (unsigned int)VirtualKey::KeyZ)
    {
        unsigned int let = k - (unsigned int)VirtualKey::KeyA;
        return ((shift != caps) ? upperLetters[let] : lowerLetters[let]);
    }
    else if(vk == VirtualKey::Space)
        return ' ';
    else if(vk == VirtualKey::Return)
        return '\n';
    else if(vk == VirtualKey::OEMMinus)
        return (shift ? '_' : '-');
    else if(vk == VirtualKey::OEMPlus)
        return (shift ? '+' : '=');
    else if(vk == VirtualKey::OEMComma)
        return (shift ? '<' : ',');
    else if(vk == VirtualKey::OEMPeriod)
        return (shift ? '>' : '.');
    else if(vk == VirtualKey::OEM1)
        return (shift ? ':' : ';');
    else if(vk == VirtualKey::OEM2)
        return (shift ? '?' : '/');
    else if(vk == VirtualKey::OEM3)
        return (shift ? '~' : '`');
    else if(vk == VirtualKey::OEM4)
        return (shift ? '{' : '[');
    else if(vk == VirtualKey::OEM5)
        return (shift ? '|' : '\\');
    else if(vk == VirtualKey::OEM6)
        return (shift ? '}' : ']');
    else if(vk == VirtualKey::OEM7)
        return (shift ? '"' : '\'');
    else if(vk == VirtualKey::Subtract)
        return '-';
    else if(vk == VirtualKey::Add)
        return '+';
    else if(vk == VirtualKey::Multiply)
        return '*';
    else if(vk == VirtualKey::Divide)
        return '/';
    else if(vk == VirtualKey::Decimal)
        return '.';
    else if(vk == VirtualKey::Back)
        return '\b';
    else if(vk == VirtualKey::Escape)
        return 0x1B;
    return 0;
}

extern "C" int kmain(multiboot_info_t *mbootInfo)
{
    MultibootInfo = mbootInfo;
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
    InputDevice::Initialize();
    PS2Keyboard::Initialize();
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

    // set kernel process root directory
    if(File *rootDir = File::Open("0:/", O_DIRECTORY))
    {
        // hijack f's DEntry
        Thread::GetCurrent()->Process->CurrentDirectory = FileSystem::GetDEntry(rootDir->DEntry);
        delete rootDir;
    }

    // load modules
    ELF::Initialize("WOOT_OS:/woot");
    Process *kernelProcess = Process::GetCurrent();
    if(File *f = File::Open("WOOT_OS:/modulelist", O_RDONLY))
    {
        FileStream fs(f);
        char line[128];
        while(fs.ReadLine(line, sizeof(line) - 1) > 0)
        {
            if(line[0] == '#' || !line[0])
                continue;
            ELF *module = ELF::Load(line);
            if(!module)
            {
                printf("[main] Couldn't load module '%s'\n", line);
                continue;
            }
            kernelProcess->Images.Append(module);
            int res = module->EntryPoint();
            printf("[main] module '%s' returned %d\n", line, res);
        }
        delete f;
    } else printf("[main] Couldn't open modulelist\n");

    debugStream.SetFrameBuffer(FrameBuffer::GetByID(0, false));

    Thread *t1 = new Thread("test 1", nullptr, (void *)testThread, 1, 0, 0, nullptr, nullptr);
    Thread *t2 = new Thread("test 2", nullptr, (void *)testThread, 2, 0, 0, nullptr, nullptr);

    t1->Enable();
    t2->Enable();

    t1->Resume(false);
    t2->Resume(false);

    Thread *ct = Thread::GetCurrent();
    InputDevice *kbd = InputDevice::GetFirstByType(InputDevice::Type::Keyboard);
    bool shift = false, num = false, caps = false, alt = false, ctrl = false;
    char cmd[256];
    int cmdPos = 0;
    printf("\nDebug (s)hell started. Don't type help for help.\n");
    for(; kbd && !quit;)
    {
        printf("%s# ", kernelProcess->CurrentDirectory->Name);
        for(;;)
        {
            InputDevice::Event event = kbd->GetEvent(0);
            if(event.Keyboard.Key == VirtualKey::LShift ||
                    event.Keyboard.Key == VirtualKey::RShift)
            {
                shift = !event.Keyboard.Release;
                continue;
            }
            if(event.Keyboard.Key == VirtualKey::LMenu ||
                    event.Keyboard.Key == VirtualKey::RMenu)
            {
                alt = !event.Keyboard.Release;
                continue;
            }
            if(event.Keyboard.Key == VirtualKey::LControl ||
                    event.Keyboard.Key == VirtualKey::RControl)
            {
                ctrl = !event.Keyboard.Release;
                continue;
            }

            if(event.Keyboard.Release)
            {
                if(event.Keyboard.Key == VirtualKey::Capital)
                    caps = !caps;
                if(event.Keyboard.Key == VirtualKey::NumLock)
                    num = !num;
                continue;
            }
            if(ctrl && alt && event.Keyboard.Key == VirtualKey::Delete)
            {
                _outb(0x64, 0xFE);
                continue;
            }
            char chr = vkToChar(event.Keyboard.Key, shift, caps, num);
            if(!chr) continue;
            if(chr == '\b')
            {
                if(cmdPos)
                {
                    printf("\b");
                    cmd[--cmdPos] = 0;
                }
                continue;
            }
            printf("%c", chr);
            if(chr == '\n')
            {
                cmd[cmdPos++] = 0;
                break;
            }
            cmd[cmdPos++] = chr;
            if(cmdPos == (sizeof(cmd) - 1))
            {
                cmd[cmdPos++] = 0;
                break;
            }
        }
        cmdPos = 0;


        Tokenizer args(cmd, " \t", 0);
        if(!args[0]) continue;
        if(!strcmp(args[0], "quit") || !strcmp(args[0], "exit"))
        {
            quit = true;
            break;
        }
        else if(!strcmp(args[0], "help"))
            printf("Nope. For some info look around line %d in %s ;)\n", __LINE__, __FILE__);
        else if(!strcmp(args[0], "reboot") || !strcmp(args[0], "reset"))
        {
            _outb(0x64, 0xFE);
        }
        else if(!strcmp(args[0], "ls") || !strcmp(args[0], "dir"))
        {
            if(File *dir = File::Open(args[1], O_DIRECTORY))
            {
                while(DirectoryEntry *de = dir->ReadDir())
                {
                    Time::DateTime mtime;
                    Time::UnixToDateTime(de->ModifyTime, &mtime);
                    printf("%-32s %s %8lu %.4d-%.2d-%.2d %.2d:%.2d\n",
                           de->Name, S_ISDIR(de->Mode) ? "<DIR> " : "      ", de->Size,
                           mtime.Year, mtime.Month, mtime.Day,
                           mtime.Hour, mtime.Minute);
                    delete de;
                }
                delete dir;
            } else printf("[main] ls failed\n");
        }
        else if(!strcmp(args[0], "cd"))
        {
            if(!args[1])
            {
                printf("missing argument\n");
                continue;
            }
            if(File *dir = File::Open(args[1], O_DIRECTORY))
            {
                if(kernelProcess->CurrentDirectory) FileSystem::PutDEntry(kernelProcess->CurrentDirectory);
                kernelProcess->CurrentDirectory = FileSystem::GetDEntry(dir->DEntry);
                delete dir;
            } else printf("[main] cd failed\n");
        }
        else if(!strcmp(args[0], "cat") || !strcmp(args[0], "type"))
        {
            if(!args[1])
            {
                printf("missing argument\n");
                continue;
            }
            if(File *f = File::Open(args[1], O_RDONLY))
            {
                char buf[64];
                while(int br = f->Read(buf, sizeof(buf)))
                {
                    if(br <= 0) break;
                    for(int i = 0; i < br; ++i)
                        printf("%c", buf[i]);
                }
                delete f;
            } else printf("[main] cat failed\n");
        }
        else if(!strcmp(args[0], "create"))
        {
            if(!args[1])
            {
                printf("missing argument\n");
                continue;
            }
            if(File *f = File::Open("", O_DIRECTORY))
            {
                if(!f->Create(args[1], S_IFREG | 0664))
                    printf("couldn't create file '%s'\n", args[1]);
                delete f;
            } else printf("[main] create failed\n");
        }
        else if(!strcmp(args[0], "rm") || !strcmp(args[0], "del"))
        {
            if(!args[1])
            {
                printf("missing argument\n");
                continue;
            }
            if(File *f = File::Open("", O_DIRECTORY))
            {
                if(f->Remove(args[1]) < 0)
                    printf("couldn't remove file '%s'\n", args[1]);
                delete f;
            } else printf("[main] remove failed\n");
        }
        else if(!strcmp(args[0], "runtime"))
        {
            double t = Time::GetSystemUpTime();
            Time::DateTime dt;
            Time::FracUnixToDateTime(t, &dt);
            printf("%.2d:%.2d:%.2d.%02d\n", dt.Hour, dt.Minute, dt.Second, dt.Millisecond);
        }
        else if(!strcmp(args[0], "time"))
        {
            Time::DateTime dt;
            Time::UnixToDateTime(time(nullptr), &dt);
            printf("%.2d:%.2d:%.2d\n", dt.Hour, dt.Minute, dt.Second);
        }
        else if(!strcmp(args[0], "date"))
        {
            Time::DateTime dt;
            Time::UnixToDateTime(time(nullptr), &dt);
            printf("%.4d-%.2d-%.2d\n", dt.Year, dt.Month, dt.Day);
        }
        else if(!strcmp(args[0], "sym"))
        {
            if(!args[1])
            {
                printf("missing argument\n");
                continue;
            }
            Elf32_Sym *sym = kernelProcess->FindSymbol(args[1]);
            if(!sym) printf("kernel symbol '%s' not found\n", args[1]);
            else printf("%s = %#.8x\n", args[1], sym->st_value);
        }
        else if(!strcmp(args[0], "grad"))
        {
            FrameBuffer *fb = FrameBuffer::GetByID(0, true);
            if(fb)
            {
                FrameBuffer::ModeInfo mode = fb->GetMode();
                for(int y = 0; y < mode.Height; ++y)
                {
                    FrameBuffer::Color c = FrameBuffer::Color::FromFloatRGB(0, 0, (float)y / mode.Height);
                    for(int x = 0; x < mode.Width; ++x)
                        fb->SetPixel(x, y, c);
                }
                fb->UnLock();
            }
        }
        else printf("Unknown command '%s'\n", args[0]);
    }
    if(!kbd) printf("[main] no keyboard\n");
    printf("[main] Closing system\n");

    t1->Finished->Wait(0, false);
    t2->Finished->Wait(0, false);

    delete t1;
    delete t2;

    FileSystem::PutDEntry(kernelProcess->CurrentDirectory);

    // call module cleanup functions in reverse order
    for(int i = kernelProcess->Images.Count() - 1; i > 0; --i)
    {
        ELF *elf = kernelProcess->Images[i];
        if(!elf || !elf->CleanupProc)
            continue;
        elf->CleanupProc();
    }

    FileSystem::SynchronizeAll();
    Volume::FlushAll();
    Volume::Cleanup();
    Drive::Cleanup();
    PS2Keyboard::Cleanup();
    InputDevice::Cleanup();
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
