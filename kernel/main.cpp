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
#include <malloc.h>
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
#include <syscalls.h>
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

extern "C" void *_utext_start;
extern "C" void *_utext_end;
extern "C" void *_udata_start;
extern "C" void *_udata_end;
extern "C" void *_ubss_start;
extern "C" void *_ubss_end;

static byte userStack[65536] USERBSS;
void USERCODE userTest()
{
    for(int a = 0; ; ++a)
    {
        asm("int $0x80":: "a"(a + 1000));
        asm("int $0x80":: "a"(123));
    }
}

void mapUser(uintptr_t as, void *start, void *end)
{
    uintptr_t ustart = (uintptr_t)start;
    uintptr_t uend = (uintptr_t)end;
    size_t usize = uend - ustart;
    size_t upages = usize / PAGE_SIZE;
    Paging::MapPages(as, ustart, ustart - KERNEL_BASE, false, true, true, upages);
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

    Process *kernelProcess = Process::GetCurrent();
    GDT::MainTSS.CR3 = kernelProcess->AddressSpace;

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
    SysCalls::Initialize();

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
    if(File *f = File::Open("WOOT_OS:/modulelist", O_RDONLY))
    {
        FileStream fs(f);
        char line[128];
        while(fs.ReadLine(line, sizeof(line) - 1) > 0)
        {
            if(line[0] == '#' || !line[0])
                continue;
            ELF *module = ELF::Load(nullptr, line, false, false);
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

    mapUser(kernelProcess->AddressSpace, &_utext_start, &_utext_end);
    mapUser(kernelProcess->AddressSpace, &_udata_start, &_udata_end);
    mapUser(kernelProcess->AddressSpace, &_ubss_start, &_ubss_end);

    FrameBuffer *fb = FrameBuffer::GetByID(0, false);
    FrameBuffer::ModeInfo mode = fb->GetMode();
    debugStream.SetFrameBuffer(fb);

    Thread *t1 = new Thread("test 1", nullptr, (void *)testThread, 1, 0, 0, nullptr, nullptr);
    Thread *t2 = new Thread("test 2", nullptr, (void *)testThread, 2, 0, 0, nullptr, nullptr);

    t1->Enable();
    t2->Enable();

    t1->Resume(false);
    t2->Resume(false);

    Thread *ct = Thread::GetCurrent();
    bool shift = false, num = false, caps = false, alt = false, ctrl = false;
    char cmd[256];
    char cwd[256];
    int cmdPos = 0;
    int x = 0, y = 0;
    DEntry *cdir = Process::GetCurrentDir();
    printf("\nDebug (s)hell started. Don't type help for help.\n");
    for(; !quit;)
    {
        cdir = Process::GetCurrentDir();
        if(cdir) cdir->GetFullPath(cwd, sizeof(cwd));
        else cwd[0] = 0;
        printf("%s# ", cwd);
        for(;;)
        {
            InputDevice::Event event = InputDevice::GetEvent(0);
            if(event.DeviceType != InputDevice::Type::Keyboard)
            {
                if(event.DeviceType == InputDevice::Type::Mouse)
                {
                    /*printf("dx: %4d dy: %4d pr: %x rel: %x hld: %x\n",
                           event.Mouse.Movement[0], event.Mouse.Movement[1],
                            event.Mouse.ButtonsPressed, event.Mouse.ButtonsReleased,
                            event.Mouse.ButtonsHeld);*/
                    x += event.Mouse.Movement[0];
                    y += event.Mouse.Movement[1];
                    x = clamp(x, 0, mode.Width - 1);
                    y = clamp(y, 0, mode.Height - 1);
                    if(event.Mouse.ButtonsHeld & 1)
                        fb->SetPixel(x, y, FrameBuffer::Color(255, 255, 255));
                }
                continue;
            }
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
                if(cdir) FileSystem::PutDEntry(cdir);
                kernelProcess->CurrentDirectory = cdir = FileSystem::GetDEntry(dir->DEntry);
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
        else if(!strcmp(args[0], "gp"))
        {
            if(!args[1] || !args[2])
                printf("missing argument\n");
            else
            {
                FrameBuffer *fb = FrameBuffer::GetByID(0, true);
                if(fb)
                {
                    int x = strtol(args[1], nullptr, 0);
                    int y = strtol(args[2], nullptr, 0);
                    FrameBuffer::Color c = fb->GetPixel(x, y);
                    printf("A: %u R: %u G: %u B: %u\n", c.A, c.R, c.G, c.B);
                    fb->UnLock();
                }
            }
        }
        else if(!strcmp(args[0], "sp"))
        {
            if(!args[1] || !args[2] || !args[3])
                printf("missing argument\n");
            else
            {
                FrameBuffer *fb = FrameBuffer::GetByID(0, true);
                if(fb)
                {
                    int x = strtol(args[1], nullptr, 0);
                    int y = strtol(args[2], nullptr, 0);
                    FrameBuffer::Color c;
                    if(!args[4] || !args[5])
                        c.Value = strtoul(args[3], nullptr, 0);
                    else
                    {
                        c.R = strtol(args[3], nullptr, 0);
                        c.G = strtol(args[4], nullptr, 0);
                        c.B = strtol(args[5], nullptr, 0);
                    }
                    fb->SetPixel(x, y, c);
                    fb->UnLock();
                }
            }
        }
        else if(!strcmp(args[0], "frac"))
        {
            struct complex
            {
                float re, im;
            };
            auto complexMul = [](complex a, complex b) -> complex
            {
                return complex { a.re * b.re - a.im * b.im, a.re * b.im + a.im * b.re };
            };
            auto complexAdd = [](complex a, complex b) -> complex
            {
                return complex { a.re * b.re, a.im + b.im };
            };

            FrameBuffer *fb = FrameBuffer::GetByID(0, true);
            if(fb)
            {
                complex con = { args[1] ? (float)strtod(args[1], nullptr) : 0.7f, args[2] ? (float)strtod(args[2], nullptr) : 0.9f };
                FrameBuffer::ModeInfo mode = fb->GetMode();
                int cx = mode.Width / 2;
                int cy = mode.Height / 2;
                for(int y = 0; y < mode.Height; ++y)
                {
                    float u = (y - cy) / (float)(cy);
                    for(int x = 0; x < mode.Width; ++x)
                    {
                        float v = (x - cx) / (float)(cx);
                        complex z = { u, v };
                        for(int i = 0; i < 15; ++i)
                        {
                            z = complexAdd(complexMul(z, z), con);
                            float mod = z.re * z.re + z.im * z.im;
                            if(mod > 10000)
                                break;
                        }
                        float mod = z.re * z.re + z.im * z.im;
                        FrameBuffer::Color c = FrameBuffer::Color::FromFloatRGB(0, 0, mod);
                        fb->SetPixel(x, y, c);
                    }
                }
                fb->UnLock();
            }
        }
        else if(!strcmp(args[0], "mstat"))
            malloc_stats();
        else if(!strcmp(args[0], "utest"))
        {
            cpuEnterUserMode((uintptr_t)(userStack + sizeof(userStack)), (uintptr_t)userTest);
        }
        else
        {
            if(File *f = File::Open(kernelProcess->CurrentDirectory, args[0], O_RDONLY))
            {
                delete f;
                Semaphore finished(0);
                Process *proc = Process::Create(cmd, &finished);
                proc->Start();
                finished.Wait(0, false, false);
                delete proc;
            }
            else printf("Unknown command '%s'\n", args[0]);
        }
    }
    printf("[main] Closing system\n");

    t1->Finished->Wait(0, false, false);
    t2->Finished->Wait(0, false, false);

    delete t1;
    delete t2;

    FileSystem::PutDEntry(cdir);

    // call module cleanup functions in reverse order
    for(int i = kernelProcess->Images.Count() - 1; i > 0; --i)
    {
        ELF *elf = kernelProcess->Images[i];
        if(!elf || !elf->CleanupProc)
            continue;
        elf->CleanupProc();
    }

    // 'close' current kernel directory
    if(kernelProcess->CurrentDirectory)
        FileSystem::PutDEntry(kernelProcess->CurrentDirectory);

    SysCalls::Cleanup();
    FileSystem::SynchronizeAll();
    FileSystem::Cleanup();
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
