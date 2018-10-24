#include <cpu.h>
#include <errno.h>
#include <stdio.h>
#include <syscalls.h>
#include <sysdefs.h>
#include <time.h>

Ints::Handler SysCalls::handler = { nullptr, SysCalls::isr, nullptr };
SysCalls::Callback SysCalls::callbacks[] =
{
    [123] = sys_msleep
};

bool SysCalls::isr(Ints::State *state, void *context)
{
    uintptr_t args[] = { state->EAX, state->EBX, state->ECX, state->EDX, state->ESI, state->EDI, state->EBP };
    if(args[0] >= MAX_SYSCALLS || !callbacks[args[0]])
    {
        printf("[syscalls] unknown syscall %u\n", args[0]);
        return -EINVAL;
        return true;
    }
    state->EAX = callbacks[args[0]](args);
    return true;
}

int SysCalls::sys_msleep(uintptr_t *args)
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

