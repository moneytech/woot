#include <cpu.h>
#include <debugstream.h>
#include <errno.h>
#include <process.h>
#include <stdio.h>
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
    [SYS_getpid] = sys_getpid,
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

long SysCalls::sys_getpid(long *args) // 20
{
    return Process::GetCurrent()->ID;
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

