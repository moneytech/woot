#include <cpu.h>
#include <mutex.h>
#include <process.h>
#include <paging.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysdefs.h>
#include <thread.h>

Sequencer<pid_t> Process::id(1);
List<Process *> *Process::processList;
Mutex *Process::listLock;
uintptr_t Process::kernelAddressSpace;

void Process::Initialize()
{
    kernelAddressSpace = Paging::GetAddressSpace();
    processList = new List<Process *>();
    listLock = new Mutex();
    Thread *ct = Thread::GetCurrent();
    new Process("Main kernel process", ct, kernelAddressSpace);
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
    threads(new List<Thread *>()),
    lock(new Mutex()),
    ID(id.GetNext()),
    Name(strdup(name)),
    AddressSpace(addressSpace ? addressSpace : NewAddressSpace())
{
    AddThread(mainThread);
    listLock->Acquire(0, false);
    processList->Append(this);
}

bool Process::AddThread(Thread *thread)
{
    if(!lock->Acquire(0, false)) return false;
    threads->Append(thread);
    thread->Process = this;
    lock->Release();
    return true;
}

bool Process::RemoveThread(Thread *thread)
{
    if(!lock->Acquire(0, false)) return false;
    bool res = threads->Remove(thread, nullptr, false) != 0;
    thread->Process = nullptr;
    lock->Release();
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
    listLock->Acquire(0, false);
    processList->Remove(this, nullptr, false);
    listLock->Release();
    if(Name) free(Name);
}
