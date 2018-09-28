#include <mutex.h>
#include <process.h>
#include <paging.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <thread.h>

Sequencer<pid_t> Process::id(1);
List<Process *> *Process::processList;
Mutex *Process::listLock;

void Process::Initialize()
{
    processList = new List<Process *>();
    listLock = new Mutex();
    Thread *ct = Thread::GetCurrent();
    new Process("Main kernel process", ct, Paging::GetAddressSpace());
}

uintptr_t Process::NewAddressSpace()
{
    NOT_IMPLEMENTED
    return 0;
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

Process::~Process()
{
    listLock->Acquire(0, false);
    processList->Remove(this, nullptr, false);
    listLock->Release();
    if(Name) free(Name);
}
