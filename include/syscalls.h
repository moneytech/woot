#ifndef SYSCALLS_H
#define SYSCALLS_H

#include <ints.h>
#include <types.h>

#define MAX_SYSCALLS 512

class SysCalls
{
    typedef int (*Callback)(uintptr_t *args);

    static Ints::Handler handler;
    static Callback callbacks[MAX_SYSCALLS];
    static bool isr(Ints::State *state, void *context);

    static int sys_msleep(uintptr_t *args);
public:
    static void Initialize();
    static void Cleanup();
};

#endif // SYSCALLS_H
