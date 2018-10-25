#include <cpu.h>
#include <debugstream.h>
#include <errno.h>
#include <paging.h>
#include <process.h>
#include <stdio.h>
#include <stdlib.h>
#include <syscall.h>
#include <syscalls.h>
#include <sysdefs.h>
#include <thread.h>
#include <time.h>

#undef TIME_H
#include <../libc/include/time.h>

extern DebugStream debugStream;

Ints::Handler SysCalls::handler = { nullptr, SysCalls::isr, nullptr };
SysCalls::Callback SysCalls::callbacks[] =
{
    [SYS_exit] = sys_exit,
    [SYS_write] = sys_write,
    [SYS_time] = sys_time,
    [SYS_getpid] = sys_getpid,
    [SYS_brk] = sys_brk,
    [SYS_nanosleep] = sys_nanosleep,
    [SYS_gettid] = sys_gettid,
};

bool SysCalls::isr(Ints::State *state, void *context)
{
    long args[] = { (long)state->EAX, (long)state->EBX, (long)state->ECX, (long)state->EDX,
                    (long)state->ESI, (long)state->EDI, (long)state->EBP };
    if(args[0] >= MAX_SYSCALLS || !callbacks[args[0]])
    {
        printf("[syscalls] unknown syscall %u\n", args[0]);
        return -EINVAL;
        return true;
    }
    state->EAX = callbacks[args[0]](args);
    return true;
}

long SysCalls::sys_exit(long *args) // 1
{
    Thread::Finalize(nullptr, args[1]);
    return 0; // should never happen
}

long SysCalls::sys_write(long *args) // 4
{
    return debugStream.Write((const void *)args[2], (int64_t)args[3]);
}

long SysCalls::sys_time(long *args) // 13
{
    return time((time_t *)args[1]);
}

long SysCalls::sys_getpid(long *args) // 20
{
    return Process::GetCurrent()->ID;
}

long SysCalls::sys_brk(long *args) // 45
{
    uintptr_t brk = args[1];
    Process *cp = Process::GetCurrent();
    if(!cp) return -EINVAL;
    if(!cp->MemoryLock.Acquire(10 * 1000, false))
        return -EBUSY;
    if(!cp->MinBrk && !cp->MaxBrk && !cp->CurrentBrk)
    {   // first call
        if(brk >= (KERNEL_BASE - 0x10000000))
        {   // we leave at least 0x10000000 bytes for stacks
            cp->MemoryLock.Release();
            return -ENOMEM;
        }
        cp->MinBrk = brk;
        cp->MaxBrk = KERNEL_BASE - 0x10000000;
        cp->CurrentBrk = cp->MinBrk;
        cp->MappedBrk = cp->CurrentBrk;
    }
    else
    {
        if(brk < cp->MinBrk || brk >= cp->MaxBrk)
        {
            cp->MemoryLock.Release();
            return -ENOMEM;
        }
        uintptr_t oldBrk = cp->CurrentBrk;
        cp->CurrentBrk = brk;
        uintptr_t neededMappedBrk = align(cp->CurrentBrk, PAGE_SIZE);
        if(neededMappedBrk > cp->MappedBrk)
        {   // allocate and map some more memory
            for(uintptr_t va = cp->MappedBrk; va < neededMappedBrk; va += PAGE_SIZE)
            {
                uintptr_t pa = Paging::AllocPage();
                if(pa == ~0)
                {
                    cp->CurrentBrk = oldBrk;
                    cp->MemoryLock.Release();;
                    return -ENOMEM;
                }
                if(!Paging::MapPage(cp->AddressSpace, va, pa, false, true, true))
                {
                    cp->CurrentBrk = oldBrk;
                    cp->MemoryLock.Release();;
                    return -ENOMEM;
                }
            }
            cp->MappedBrk = neededMappedBrk;
        }
        else if(neededMappedBrk < cp->MappedBrk)
        {   // unmap and release memory
            for(uintptr_t va = neededMappedBrk; va < cp->MappedBrk; va += PAGE_SIZE)
            {
                uintptr_t pa = Paging::GetPhysicalAddress(cp->AddressSpace, va);
                if(pa != ~0)
                    Paging::FreePage(pa);
                Paging::UnMapPage(cp->AddressSpace, va, false);
            }
            cp->MappedBrk = neededMappedBrk;
        }
    }
    cp->MemoryLock.Release();
    return 0;
}

long SysCalls::sys_nanosleep(long *args) // 162
{
    struct timespec *req = (struct timespec *)args[1];
    if(!req || req->tv_sec < 0 || req->tv_nsec > 999999999)
        return -EINVAL;
    struct timespec *rem = (struct timespec *)args[2];
    uint millisReq = req->tv_sec * 1000 + req->tv_nsec / (1000 * 1000);
    uint millisLeft = Time::Sleep(millisReq, true);
    int res = 0;
    if(millisLeft)
    {
        res = -EINTR;
        if(rem)
        {
            rem->tv_sec = millisLeft / 1000;
            rem->tv_nsec = (millisLeft % 1000) * 1000 * 1000;
        }
    }
    return res;
}

long SysCalls::sys_gettid(long *args)
{
    return Thread::GetCurrent()->ID;
}

void SysCalls::Initialize()
{
    Ints::RegisterHandler(SYSCALLS_INT_VECTOR, &handler);
}

void SysCalls::Cleanup()
{
    Ints::UnRegisterHandler(SYSCALLS_INT_VECTOR, &handler);
}

