#include <cpu.h>
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
    printf("filename: %s\n", filename);
    ELF *elf = ELF::Load(GetCurrentDir(), filename, false);
    if(!elf) return 127;
    if(!elf->EntryPoint)
    {
        delete elf;
        return 126;
    }
    return elf->EntryPoint();
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
    Process *proc = new Process("filename", thread, 0);
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

Process::~Process()
{
    lock.Acquire(0, false);
    listLock.Acquire(0, false);
    if(CurrentDirectory) FileSystem::PutDEntry(CurrentDirectory);
    processList.Remove(this, nullptr, false);
    listLock.Release();
    if(Name) free(Name);
}
