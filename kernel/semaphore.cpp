#include <cpu.h>
#include <semaphore.h>
#include <stdio.h>
#include <thread.h>

// TODO: make waiters queue arbitrarily long without any heap allocations
#define MAX_WAITERS 32

Semaphore::Semaphore(int count) :
    Count(count),
    Waiters(new Queue<Thread *>(MAX_WAITERS))
{
}

bool Semaphore::Wait(uint timeout, bool tryWait)
{
    bool is = cpuDisableInterrupts();
    Thread *ct = Thread::GetCurrent();
    if(Count > 0)
    {
        --Count;
        cpuRestoreInterrupts(is);
        return true;
    }
    if(tryWait)
    {
        cpuRestoreInterrupts(is);
        return false;
    }
    if(Waiters->Write(ct))
    {
        bool r = true;

        if(timeout) r = ct->Sleep(timeout, true) != 0;
        else ct->Suspend();

        if(!r)
        {
            Thread *t = Waiters->Peek();
            if(ct == t) Waiters->Read(nullptr);
            else Waiters->ReplaceFirst(ct, nullptr);
        }
        else --Count;

        cpuRestoreInterrupts(is);
        return r;
    }
    // if no free waiter slots then print message and fail
    cpuRestoreInterrupts(is);
    printf("!!! Semaphore ran out of free waiter slots !!!\n");
    return false;
}

void Semaphore::Signal(Ints::State *state)
{
    bool is = cpuDisableInterrupts();
    ++Count;
    bool ok;
    Thread *t = nullptr;
    do
    {
        t = Waiters->Read(&ok);
        if(!ok)
        { // no waiting threads in queue
            cpuRestoreInterrupts(is);
            return;
        }
    } while(!t);

    if(state) t->QuickResume(state);
    else t->Resume(false);

    cpuRestoreInterrupts(is);
}

Semaphore::~Semaphore()
{
    if(Waiters) delete Waiters;
}
