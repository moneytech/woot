#include <cpu.h>
#include <errno.h>
#include <file.h>
#include <filesystem.h>
#include <mutex.h>
#include <process.h>
#include <paging.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysdefs.h>
#include <thread.h>

Sequencer<pid_t> Process::id(1);
List<Process *> Process::processList;
Mutex Process::listLock;
uintptr_t Process::kernelAddressSpace;

int Process::processEntryPoint(const char *filename)
{
    ELF *elf = ELF::Load(GetCurrentDir(), filename, true, false);
    if(!elf) return 127;
    if(!elf->EntryPoint)
    {
        delete elf;
        return 126;
    }
    Process *proc = GetCurrent();
    if(!proc->lock.Acquire(0, false))
    {
        delete elf;
        return 126;
    }

    proc->MemoryLock.Acquire(0, false);
    for(ELF *e : proc->Images)
        proc->MinBrk = max(proc->MinBrk, e->GetEndPtr());
    proc->CurrentBrk = proc->MinBrk;
    proc->MappedBrk = proc->CurrentBrk;
    proc->MaxBrk = KERNEL_BASE - 0x10000000;
    proc->MemoryLock.Release();

    uintptr_t esp = Thread::GetCurrent()->AllocUserStack();
    proc->lock.Release();
    cpuEnterUserMode(esp, (uintptr_t)elf->EntryPoint);
    return 0;
}

void Process::Initialize()
{
    kernelAddressSpace = Paging::GetAddressSpace();
    Thread *ct = Thread::GetCurrent();
    new Process("Main kernel process", ct, kernelAddressSpace);
}

Process *Process::Create(const char *filename, Semaphore *finished)
{
    if(!filename) return nullptr;
    Thread *thread = new Thread("main", nullptr, (void *)processEntryPoint, (uintptr_t)filename,
                                DEFAULT_STACK_SIZE, DEFAULT_STACK_SIZE,
                                nullptr, finished);
    Process *proc = new Process(filename, thread, 0);
    return proc;
}

Process *Process::GetCurrent()
{
    Thread *ct = Thread::GetCurrent();
    if(!ct) return nullptr;
    return ct->Process;
}

DEntry *Process::GetCurrentDir()
{
    bool ints = cpuDisableInterrupts();
    Thread *ct = Thread::GetCurrent();
    if(!ct)
    {
        cpuRestoreInterrupts(ints);
        return nullptr;
    }
    Process *cp = ct->Process;
    if(!cp)
    {
        cpuRestoreInterrupts(ints);
        return nullptr;
    }
    DEntry *de = cp->CurrentDirectory;
    cpuRestoreInterrupts(ints);
    return de;
}

uintptr_t Process::NewAddressSpace()
{
    uintptr_t newAS = Paging::AllocPage();
    if(newAS == ~0)
        return ~0;
    Paging::BuildAddressSpace(newAS);
    return newAS;
}

void Process::Cleanup()
{
    NOT_IMPLEMENTED
}

Process::Process(const char *name, Thread *mainThread, uintptr_t addressSpace) :
    UserStackPtr(KERNEL_BASE),
    ID(id.GetNext()),
    Name(strdup(name)),
    AddressSpace(addressSpace ? addressSpace : NewAddressSpace())
{
    DEntry *cdir = GetCurrentDir();
    if(cdir) CurrentDirectory = FileSystem::GetDEntry(cdir);
    AddThread(mainThread);
    listLock.Acquire(0, false);
    processList.Append(this);
    listLock.Release();    
}

bool Process::Start()
{
    if(!lock.Acquire(0, false))
        return false;
    Thread *t = threads[0];
    if(!t) return false;
    t->Enable();
    bool res = t->Resume(false);
    lock.Release();
    return res;
}

bool Process::AddThread(Thread *thread)
{
    if(!lock.Acquire(0, false)) return false;
    threads.Append(thread);
    thread->Process = this;
    lock.Release();
    return true;
}

bool Process::RemoveThread(Thread *thread)
{
    if(!lock.Acquire(0, false)) return false;
    bool res = threads.Remove(thread, nullptr, false) != 0;
    thread->Process = nullptr;
    lock.Release();
    return res;
}

Elf32_Sym *Process::FindSymbol(const char *name)
{
    for(ELF *elf : Images)
    {
        Elf32_Sym *sym = elf->FindSymbol(name);
        if(sym) return sym;
    }
    return nullptr;
}

int Process::Open(const char *filename, int flags)
{
    if(!lock.Acquire(0, false))
        return -EBUSY;
    File *f = File::Open(filename, flags);
    if(!f)
    {
        lock.Release();
        return -ENOENT;
    }
    int fd = 3;
    for(; fd < MAX_FILE_DESCRIPTORS; ++fd)
    {
        if(!FileDescriptors[fd])
        {
            FileDescriptors[fd] = f;
            break;
        }
    }
    if(fd >= MAX_FILE_DESCRIPTORS)
        fd = -EMFILE;
    lock.Release();
    return fd;
}

int Process::Close(int fd)
{
    if(fd < 0 || fd >= MAX_FILE_DESCRIPTORS)
        return -EBADF;
    if(!lock.Acquire(0, false))
        return -EBUSY;
    File *f = FileDescriptors[fd];
    if(!f)
    {
        lock.Release();
        return -EBADF;
    }
    delete f;
    FileDescriptors[fd] = nullptr;
    lock.Release();
    return 0;
}

File *Process::GetFileDescriptor(int fd)
{
    if(fd < 0 || fd >= MAX_FILE_DESCRIPTORS)
        return nullptr;
    if(!lock.Acquire(0, false))
        return nullptr;
    File *f = FileDescriptors[fd];
    lock.Release();
    return f;
}

Process::~Process()
{
    lock.Acquire(0, false);
    listLock.Acquire(0, false);
    for(Thread *t : threads)
    {
        if(t->State != Thread::State::Finalized)
            Thread::Finalize(t, -1);
        delete t;
    }
    for(ELF *elf : Images)
        if(elf) delete elf;
    Images.Clear();
    if(CurrentDirectory) FileSystem::PutDEntry(CurrentDirectory);
    processList.Remove(this, nullptr, false);
    listLock.Release();
    if(Name) free(Name);
}
