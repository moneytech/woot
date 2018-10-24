#include <cpu.h>
#include <debugstream.h>
#include <errno.h>
#include <stdio.h>
#include <syscalls.h>
#include <sysdefs.h>
#include <thread.h>
#include <time.h>

extern DebugStream debugStream;

Ints::Handler SysCalls::handler = { nullptr, SysCalls::isr, nullptr };
SysCalls::Callback SysCalls::callbacks[] =
{
    [1] = sys_exit,
    [4] = sys_write,
    [123] = sys_msleep
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

long SysCalls::sys_msleep(long *args)
{
    return Time::Sleep(args[1], false);
}

void SysCalls::Initialize()
{
    Ints::RegisterHandler(SYSCALLS_INT_VECTOR, &handler);
}

void SysCalls::Cleanup()
{
    Ints::UnRegisterHandler(SYSCALLS_INT_VECTOR, &handler);
}

