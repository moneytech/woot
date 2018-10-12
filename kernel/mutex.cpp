#include <mutex.h>
#include <stdio.h>
#include <thread.h>
#include <time.h>

// TODO: make waiters queue arbitrarily long without any heap allocations
#define MAX_WAITERS 32

// global kernel lock mutex
Mutex Mutex::GlobalLock;

Mutex::Mutex() :
    Count(0), Owner(nullptr),
    Waiters(new Queue<Thread *>(MAX_WAITERS))
{
}

bool Mutex::Acquire(uint timeout, bool tryAcquire)
{
    bool is = cpuDisableInterrupts();
    Thread *ct = Thread::GetCurrent();
    if(!Count || Owner == ct)
    {
        ++Count;
        Owner = ct;
        cpuRestoreInterrupts(is);
        return true;
    }
    if(tryAcquire)
    {
        cpuRestoreInterrupts(is);
        return false;
    }
    if(Waiters->Write(ct))
    {
        bool r = true;

        ct->WaitingMutex = this;
        if(timeout) r = ct->Sleep(timeout, true) != 0;
        else ct->Suspend();
        ct->WaitingMutex = nullptr;

        if(!r)
        {
            Thread *t = Waiters->Peek();
            if(ct == t) Waiters->Read(nullptr);
            else Waiters->ReplaceFirst(ct, nullptr);
        }

        cpuRestoreInterrupts(is);
        return r;
    }
    // if no free waiter slots then print message and fail
    cpuRestoreInterrupts(is);
    printf("!!! Mutex ran out of free waiter slots !!!\n");
    return false;
}

void Mutex::Release()
{
    bool is = cpuDisableInterrupts();
    Thread *ct = Thread::GetCurrent();
    if(Owner != ct)
    {
        printf("[mutex] Mutex::Release(): current thread(%d) != Owner(%d)\n", ct->ID, Owner ? Owner->ID : -1);
        cpuRestoreInterrupts(is);
        return;
    }
    if(Count > 0)
        --Count;
    if(!Count)
    {
        Owner = nullptr;
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

        // make last unqueued thread an owner
        Owner = t;
        Count = 1;
        t->Resume(false);
    }
    cpuRestoreInterrupts(is);
}

void Mutex::Cancel(Thread *t)
{
    bool is = cpuDisableInterrupts();
    Waiters->ReplaceAll(t, nullptr);
    cpuRestoreInterrupts(is);
}

Mutex::~Mutex()
{
    if(Waiters) delete Waiters;
}
